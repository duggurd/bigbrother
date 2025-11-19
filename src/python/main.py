import os
import sys
import time
import json
import psutil
import win32gui
import win32process
import threading
import tkinter as tk
from tkinter import ttk, messagebox, simpledialog
from datetime import datetime
from pathlib import Path

# --- Configuration ---
POLL_INTERVAL = 1.0  # Seconds between checks
DATA_DIR_NAME = "BigBrother"
DATA_FILE_NAME = "focus_log.json"

class SessionLogger:
    def __init__(self):
        self.session_start = 0
        self.session_active = False
        self.session_comment = ""
        self.data_file_path = self._get_data_file_path()
        
        self.current_process_name = ""
        self.current_process_path = ""
        self.current_window_title = ""
        self.current_focus_start_time = 0
        
        self.applications = {}  # key: process_name, value: dict
        
        # For Polling
        self.last_hwnd = 0
        self.last_title = ""
        
        # Threading control
        self._stop_event = threading.Event()
        self._monitor_thread = None

    def _get_data_file_path(self):
        app_data = os.getenv('APPDATA')
        if not app_data:
            app_data = os.path.expanduser("~")
        
        base_path = os.path.join(app_data, DATA_DIR_NAME)
        os.makedirs(base_path, exist_ok=True)
        return os.path.join(base_path, DATA_FILE_NAME)

    def start_session(self, comment=""):
        if self.session_active:
            return False
            
        self.session_start = int(time.time())
        self.session_active = True
        self.session_comment = comment
        self.applications = {}
        self.current_process_name = ""
        self.current_process_path = ""
        self.current_window_title = ""
        self.current_focus_start_time = 0
        self.last_hwnd = 0
        self.last_title = ""
        
        self._stop_event.clear()
        self._monitor_thread = threading.Thread(target=self._monitor_loop, daemon=True)
        self._monitor_thread.start()
        
        print(f"Session started: {comment}")
        return True

    def stop_session(self):
        if not self.session_active:
            return

        self._stop_event.set()
        if self._monitor_thread:
            self._monitor_thread.join(timeout=2.0)
            
        self.finalize_focus(int(time.time()))
        self.flush_to_disk()
        
        self.session_active = False
        print("Session stopped.")

    def delete_session(self, session_timestamp):
        """Deletes a session from the JSON file based on its start timestamp."""
        if not os.path.exists(self.data_file_path):
            return False

        try:
            # Read current data
            with open(self.data_file_path, 'r', encoding='utf-8') as f:
                content = f.read().strip()
                if not content: return False
                data = json.loads(content)
            
            sessions = data.get("sessions", [])
            initial_count = len(sessions)
            
            # Filter out the session with matching timestamp
            filtered_sessions = [s for s in sessions if s.get("start_timestamp") != session_timestamp]
            
            if len(filtered_sessions) == initial_count:
                return False # Session not found
                
            data["sessions"] = filtered_sessions
            
            # Write back
            temp_path = self.data_file_path + ".tmp"
            with open(temp_path, 'w', encoding='utf-8') as f:
                json.dump(data, f, indent=2)
            
            if os.path.exists(self.data_file_path):
                os.remove(self.data_file_path)
            os.rename(temp_path, self.data_file_path)
            
            return True
            
        except Exception as e:
            print(f"Error deleting session: {e}")
            return False

    def _monitor_loop(self):
        while not self._stop_event.is_set():
            self.update()
            time.sleep(POLL_INTERVAL)

    def _get_active_window_info(self):
        try:
            hwnd = win32gui.GetForegroundWindow()
            if not hwnd:
                return None, None, None, None

            window_title = win32gui.GetWindowText(hwnd)
            _, pid = win32process.GetWindowThreadProcessId(hwnd)
            
            try:
                process = psutil.Process(pid)
                process_name = process.name()
                process_path = process.exe()
            except (psutil.NoSuchProcess, psutil.AccessDenied):
                process_name = f"Unknown/System (PID: {pid})"
                process_path = "Unknown"

            return hwnd, window_title, process_name, process_path
        except Exception:
            return None, None, None, None

    def update(self):
        hwnd, title, proc_name, proc_path = self._get_active_window_info()
        if not hwnd: return

        if hwnd != self.last_hwnd or title != self.last_title:
            current_time = int(time.time())
            
            if self.current_focus_start_time > 0:
                self.finalize_focus(current_time)
                self.flush_to_disk() 

            self.current_process_name = proc_name
            self.current_process_path = proc_path
            self.current_window_title = title
            self.current_focus_start_time = current_time
            
            self.last_hwnd = hwnd
            self.last_title = title

    def finalize_focus(self, end_time):
        if not self.current_process_name: return

        duration_ms = (end_time - self.current_focus_start_time) * 1000
        if duration_ms <= 0: return

        if self.current_process_name not in self.applications:
            self.applications[self.current_process_name] = {
                "process_name": self.current_process_name,
                "process_path": self.current_process_path,
                "first_focus_time": self.current_focus_start_time,
                "last_focus_time": end_time,
                "total_time_ms": 0,
                "tabs": {}
            }
        
        app_data = self.applications[self.current_process_name]
        app_data["last_focus_time"] = end_time
        app_data["total_time_ms"] += duration_ms
        
        if self.current_window_title not in app_data["tabs"]:
            app_data["tabs"][self.current_window_title] = {
                "window_title": self.current_window_title,
                "total_time_spent_ms": 0
            }
        app_data["tabs"][self.current_window_title]["total_time_spent_ms"] += duration_ms

    def build_json(self):
        session_data = {
            "start_timestamp": self.session_start,
            "end_timestamp": int(time.time()) if self.session_active else self.session_start,
            "comment": self.session_comment,
            "applications": []
        }
        
        for app_name, app_info in self.applications.items():
            tabs_list = list(app_info["tabs"].values())
            clean_app = {
                "process_name": app_info["process_name"],
                "process_path": app_info["process_path"],
                "first_focus_time": app_info["first_focus_time"],
                "last_focus_time": app_info["last_focus_time"],
                "total_time_spent_ms": app_info["total_time_ms"],
                "tabs": tabs_list
            }
            session_data["applications"].append(clean_app)
        return session_data

    def flush_to_disk(self):
        all_data = {"sessions": []}
        if os.path.exists(self.data_file_path):
            try:
                with open(self.data_file_path, 'r', encoding='utf-8') as f:
                    content = f.read().strip()
                    if content: all_data = json.loads(content)
            except Exception: pass

        if "sessions" not in all_data: all_data["sessions"] = []

        current_session_json = self.build_json()
        found = False
        for i, session in enumerate(all_data["sessions"]):
            if session.get("start_timestamp") == self.session_start:
                all_data["sessions"][i] = current_session_json
                found = True
                break
        
        if not found:
            all_data["sessions"].append(current_session_json)
            
        try:
            temp_path = self.data_file_path + ".tmp"
            with open(temp_path, 'w', encoding='utf-8') as f:
                json.dump(all_data, f, indent=2)
            if os.path.exists(self.data_file_path):
                os.remove(self.data_file_path)
            os.rename(temp_path, self.data_file_path)
        except Exception as e:
            print(f"Error saving: {e}")


class MainApp:
    def __init__(self, root):
        self.root = root
        self.root.title("BigBrother - Activity Monitor")
        self.root.geometry("1100x700")
        
        self.logger = SessionLogger()
        
        # Keep track of expanded nodes to restore state after refresh
        self.expanded_nodes = set()
        
        # UI Setup
        self._setup_ui()
        self._load_historical_data()
        
        # Start auto-refresh timer for UI
        self.root.after(1000, self._ui_refresh_loop)

    def _setup_ui(self):
        # Top Control Panel
        control_frame = ttk.Frame(self.root, padding=10)
        control_frame.pack(fill=tk.X)
        
        self.btn_start = ttk.Button(control_frame, text="Start New Session", command=self._on_start_session)
        self.btn_start.pack(side=tk.LEFT, padx=5)
        
        self.btn_stop = ttk.Button(control_frame, text="Stop Session", command=self._on_stop_session, state=tk.DISABLED)
        self.btn_stop.pack(side=tk.LEFT, padx=5)
        
        self.lbl_status = ttk.Label(control_frame, text="Status: Idle", font=("Segoe UI", 10, "bold"))
        self.lbl_status.pack(side=tk.LEFT, padx=20)
        
        ttk.Button(control_frame, text="Refresh View", command=self._load_historical_data).pack(side=tk.RIGHT, padx=5)

        # Main Tree View
        tree_frame = ttk.Frame(self.root, padding=10)
        tree_frame.pack(fill=tk.BOTH, expand=True)
        
        # Columns: Add 'action' column for delete button
        columns = ("duration", "percent", "start", "end", "comment", "action")
        self.tree = ttk.Treeview(tree_frame, columns=columns, selectmode="browse")
        
        self.tree.heading("#0", text="Name / Title", anchor=tk.W)
        self.tree.heading("duration", text="Time Spent", anchor=tk.E)
        self.tree.heading("percent", text="%", anchor=tk.E)
        self.tree.heading("start", text="Start Time", anchor=tk.W)
        self.tree.heading("end", text="End Time", anchor=tk.W)
        self.tree.heading("comment", text="Comment", anchor=tk.W)
        self.tree.heading("action", text="Action", anchor=tk.CENTER)

        self.tree.column("#0", minwidth=300, width=350)
        self.tree.column("duration", width=80, anchor=tk.E)
        self.tree.column("percent", width=60, anchor=tk.E)
        self.tree.column("start", width=130)
        self.tree.column("end", width=130)
        self.tree.column("comment", width=200)
        self.tree.column("action", width=50, anchor=tk.CENTER) # Small column for the X

        scrollbar = ttk.Scrollbar(tree_frame, orient=tk.VERTICAL, command=self.tree.yview)
        self.tree.configure(yscroll=scrollbar.set)
        
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)
        self.tree.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # Bind events
        self.tree.bind("<<TreeviewOpen>>", self._on_tree_open)
        self.tree.bind("<Button-1>", self._on_click) # Capture clicks for custom button logic
        self.tree.bind("<Button-3>", self._show_context_menu)
        
        # Create context menu
        self.context_menu = tk.Menu(self.root, tearoff=0)
        self.context_menu.add_command(label="Delete Session", command=self._delete_selected_session)

    def _on_tree_open(self, event):
        pass
        
    def _on_click(self, event):
        # Check if click is on the "Action" column
        region = self.tree.identify("region", event.x, event.y)
        if region == "cell":
            col = self.tree.identify_column(event.x)
            # columns are #1, #2... corresponds to display columns
            # "action" is the 6th column in values tuple, so it should be #6
            if col == "#6": 
                item_id = self.tree.identify_row(event.y)
                if item_id and item_id.startswith("S_") and not "_A_" in item_id:
                    # Trigger delete logic
                    self.tree.selection_set(item_id)
                    self._delete_selected_session()
    
    def _show_context_menu(self, event):
        item = self.tree.identify_row(event.y)
        if item:
            self.tree.selection_set(item)
            if item.startswith("S_") and not "_A_" in item: 
                self.context_menu.post(event.x_root, event.y_root)
                
    def _delete_selected_session(self):
        selected = self.tree.selection()
        if not selected: return
        
        item_id = selected[0]
        if not (item_id.startswith("S_") and not "_A_" in item_id):
            return
            
        try:
            ts_str = item_id.split("_")[1]
            timestamp = int(ts_str)
            
            if messagebox.askyesno("Delete Session", "Are you sure you want to delete this session permanently?"):
                if self.logger.delete_session(timestamp):
                    self._load_historical_data()
                else:
                    messagebox.showerror("Error", "Failed to delete session.")
        except Exception as e:
            print(f"Error parsing session ID for deletion: {e}")

    def _on_start_session(self):
        comment = simpledialog.askstring("New Session", "Enter session description/goal:", parent=self.root)
        if comment is None: return
        if not comment.strip():
            messagebox.showwarning("Required", "A description is required to start a session.")
            return

        if self.logger.start_session(comment):
            self.btn_start.config(state=tk.DISABLED)
            self.btn_stop.config(state=tk.NORMAL)
            self.lbl_status.config(text=f"Status: Recording ({comment})", foreground="green")
            self._load_historical_data()

    def _on_stop_session(self):
        if messagebox.askyesno("Confirm", "Stop current session?"):
            self.logger.stop_session()
            self.btn_start.config(state=tk.NORMAL)
            self.btn_stop.config(state=tk.DISABLED)
            self.lbl_status.config(text="Status: Idle", foreground="black")
            self._load_historical_data()

    def _ui_refresh_loop(self):
        if self.logger.session_active:
            self._load_historical_data()
        self.root.after(2000, self._ui_refresh_loop)

    def _format_duration(self, ms):
        seconds = int(ms / 1000)
        if seconds < 60: return f"{seconds}s"
        minutes = seconds // 60
        if minutes < 60: return f"{minutes}m {seconds%60}s"
        hours = minutes // 60
        return f"{hours}h {minutes%60}m"

    def _format_timestamp(self, ts):
        if ts == 0: return ""
        return datetime.fromtimestamp(ts).strftime('%Y-%m-%d %H:%M:%S')

    def _load_historical_data(self):
        current_expanded = set()
        for item in self.tree.get_children():
            if self.tree.item(item, "open"):
                current_expanded.add(item)
            for child in self.tree.get_children(item):
                if self.tree.item(child, "open"):
                    current_expanded.add(child)
        
        if not os.path.exists(self.logger.data_file_path):
            return

        try:
            with open(self.logger.data_file_path, 'r', encoding='utf-8') as f:
                content = f.read().strip()
                data = json.loads(content) if content else {"sessions": []}
        except Exception:
            return

        sessions = data.get("sessions", [])
        sessions.sort(key=lambda x: x.get("start_timestamp", 0), reverse=True)

        selected_item = self.tree.focus()
        self.tree.delete(*self.tree.get_children())

        for session in sessions:
            start_ts = session.get("start_timestamp", 0)
            end_ts = session.get("end_timestamp", 0)
            comment = session.get("comment", "")
            apps = session.get("applications", [])
            
            total_ms = sum(a.get("total_time_spent_ms", 0) for a in apps)
            date_str = datetime.fromtimestamp(start_ts).strftime('%Y-%m-%d %H:%M')
            
            display_text = f"Session: {date_str}"
            if start_ts == self.logger.session_start and self.logger.session_active:
                display_text += " (ACTIVE)"

            session_id = f"S_{start_ts}"
            is_open = (session_id in current_expanded) or (start_ts == self.logger.session_start)

            # Only show [X] for the session row
            action_text = "❌" 

            self.tree.insert(
                "", 
                tk.END, 
                iid=session_id,
                text=display_text,
                values=(
                    self._format_duration(total_ms),
                    "100%",
                    self._format_timestamp(start_ts),
                    self._format_timestamp(end_ts),
                    comment,
                    action_text # Action column value
                ),
                open=is_open
            )

            apps.sort(key=lambda x: x.get("total_time_spent_ms", 0), reverse=True)
            
            for app in apps:
                app_name = app.get("process_name", "Unknown")
                app_ms = app.get("total_time_spent_ms", 0)
                percent = (app_ms / total_ms * 100) if total_ms > 0 else 0
                
                app_id = f"{session_id}_A_{app_name}"
                is_app_open = app_id in current_expanded
                if self.tree.exists(app_id):
                    app_id = f"{app_id}_{app.get('first_focus_time', 0)}"

                self.tree.insert(
                    session_id,
                    tk.END,
                    iid=app_id,
                    text=app_name,
                    values=(
                        self._format_duration(app_ms),
                        f"{percent:.1f}%",
                        "", "", "", "" # Empty action for children
                    ),
                    open=is_app_open
                )
                
                for i, tab in enumerate(sorted(app.get("tabs", []), key=lambda x: x.get("total_time_spent_ms", 0), reverse=True)):
                    tab_ms = tab.get("total_time_spent_ms", 0)
                    tab_pct = (tab_ms / app_ms * 100) if app_ms > 0 else 0
                    
                    tab_id = f"{app_id}_T_{i}"
                    
                    self.tree.insert(
                        app_id,
                        tk.END,
                        iid=tab_id,
                        text=tab.get("window_title", "Unknown"),
                        values=(
                            self._format_duration(tab_ms),
                            f"{tab_pct:.1f}%",
                            "", "", "", ""
                        )
                    )

        if selected_item and self.tree.exists(selected_item):
            self.tree.selection_set(selected_item)
            self.tree.see(selected_item)

if __name__ == "__main__":
    root = tk.Tk()
    try:
        style = ttk.Style()
        style.theme_use('clam')
    except: pass
    
    app = MainApp(root)
    root.mainloop()
