#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <shlobj.h>
#include "json.hpp"

using json = nlohmann::json;

// DirectX 11 data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// Forward declarations
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Data structures
struct TitleChange {
    long long timestamp;
    std::string title;
};

struct WindowFocusEvent {
    long long focus_timestamp;
    std::string window_title;
    std::string process_name;
    std::string process_path;
    std::vector<TitleChange> title_changes;
    bool expanded = false;
};

struct Session {
    long long start_timestamp;
    long long end_timestamp;
    std::vector<WindowFocusEvent> window_focus;
    bool expanded = false;
};

std::vector<Session> g_sessions;
std::string g_dataFilePath;
int g_selectedSession = -1;

// Helper function to get user data path
std::string GetUserDataPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        return std::string(path) + "\\BigBrother\\focus_log.json";
    }
    return "focus_log.json";
}

// Helper function to format timestamp
std::string FormatTimestamp(long long timestamp) {
    if (timestamp == 0) return "N/A";
    
    time_t rawtime = (time_t)timestamp;
    struct tm timeinfo;
    localtime_s(&timeinfo, &rawtime);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buffer);
}

// Helper function to format time only
std::string FormatTime(long long timestamp) {
    if (timestamp == 0) return "N/A";
    
    time_t rawtime = (time_t)timestamp;
    struct tm timeinfo;
    localtime_s(&timeinfo, &rawtime);
    
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", &timeinfo);
    return std::string(buffer);
}

// Helper function to format duration
std::string FormatDuration(long long seconds) {
    if (seconds < 60) {
        return std::to_string(seconds) + "s";
    } else if (seconds < 3600) {
        long long mins = seconds / 60;
        long long secs = seconds % 60;
        return std::to_string(mins) + "m " + std::to_string(secs) + "s";
    } else {
        long long hours = seconds / 3600;
        long long mins = (seconds % 3600) / 60;
        return std::to_string(hours) + "h " + std::to_string(mins) + "m";
    }
}

// Load sessions from JSON file
void LoadSessions() {
    g_sessions.clear();
    g_selectedSession = -1;
    
    std::ifstream inFile(g_dataFilePath);
    if (!inFile.is_open()) {
        return;
    }
    
    try {
        json data;
        inFile >> data;
        
        if (data.contains("sessions") && data["sessions"].is_array()) {
            for (const auto& sessionJson : data["sessions"]) {
                Session session;
                session.start_timestamp = sessionJson.value("start_timestamp", 0LL);
                session.end_timestamp = sessionJson.value("end_timestamp", 0LL);
                
                if (sessionJson.contains("window_focus") && sessionJson["window_focus"].is_array()) {
                    for (const auto& focusJson : sessionJson["window_focus"]) {
                        WindowFocusEvent event;
                        event.focus_timestamp = focusJson.value("focus_timestamp", 0LL);
                        event.window_title = focusJson.value("window_title", "");
                        event.process_name = focusJson.value("process_name", "");
                        event.process_path = focusJson.value("process_path", "");
                        
                        if (focusJson.contains("title_changes") && focusJson["title_changes"].is_array()) {
                            for (const auto& titleJson : focusJson["title_changes"]) {
                                TitleChange tc;
                                tc.timestamp = titleJson.value("title_timestamp", 0LL);
                                tc.title = titleJson.value("window_title", "");
                                event.title_changes.push_back(tc);
                            }
                        }
                        
                        session.window_focus.push_back(event);
                    }
                }
                
                g_sessions.push_back(session);
            }
        }
    } catch (const json::exception& e) {
        // Error parsing JSON
    }
    
    inFile.close();
}

// Main code
int main(int, char**)
{
    // Get data file path
    g_dataFilePath = GetUserDataPath();
    LoadSessions();
    
    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"BigBrother Viewer", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"BigBrother Session Viewer", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);

    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle messages
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Create main window
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("BigBrother Session Viewer", nullptr, 
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | 
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);

        // Menu bar
        if (ImGui::BeginMenuBar())
        {
            ImGui::Text("File: %s", g_dataFilePath.c_str());
            ImGui::Spacing();
            if (ImGui::Button("Reload"))
            {
                LoadSessions();
            }
            ImGui::EndMenuBar();
        }

        // Sessions List
        ImGui::Text("Sessions");
        ImGui::Separator();
        
        if (ImGui::BeginChild("SessionsList", ImVec2(0, 250), true))
        {
            for (int i = 0; i < g_sessions.size(); i++)
            {
                const auto& session = g_sessions[i];
                
                ImGui::PushID(i);
                
                // Calculate duration
                long long duration = session.end_timestamp - session.start_timestamp;
                
                // Session header
                std::string header = "Session " + std::to_string(i + 1);
                bool is_selected = (g_selectedSession == i);
                
                if (ImGui::Selectable(header.c_str(), is_selected, 0, ImVec2(0, 0)))
                {
                    g_selectedSession = i;
                }
                
                if (is_selected)
                {
                    ImGui::Indent();
                    ImGui::Text("Start:    %s", FormatTimestamp(session.start_timestamp).c_str());
                    ImGui::Text("End:      %s", FormatTimestamp(session.end_timestamp).c_str());
                    ImGui::Text("Duration: %s", FormatDuration(duration).c_str());
                    ImGui::Text("Window Changes: %d", (int)session.window_focus.size());
                    ImGui::Unindent();
                }
                
                ImGui::PopID();
            }
        }
        ImGui::EndChild();

        // Window Focus Events
        ImGui::Spacing();
        if (g_selectedSession >= 0 && g_selectedSession < g_sessions.size())
        {
            ImGui::Text("Window Focus Events (Session %d)", g_selectedSession + 1);
            ImGui::Separator();
            
            const auto& session = g_sessions[g_selectedSession];
            
            if (ImGui::BeginChild("FocusEvents", ImVec2(0, 300), true))
            {
                for (int i = 0; i < session.window_focus.size(); i++)
                {
                    auto& event = g_sessions[g_selectedSession].window_focus[i];
                    
                    ImGui::PushID(i);
                    
                    // Calculate duration (time until next event or session end)
                    long long duration = 0;
                    if (i < session.window_focus.size() - 1) {
                        duration = session.window_focus[i + 1].focus_timestamp - event.focus_timestamp;
                    } else {
                        duration = session.end_timestamp - event.focus_timestamp;
                    }
                    
                    // Event header
                    std::string time_str = FormatTime(event.focus_timestamp);
                    std::string header = time_str + " - " + event.process_name;
                    
                    if (ImGui::TreeNode(header.c_str()))
                    {
                        ImGui::Text("Title: %s", event.window_title.c_str());
                        ImGui::Text("Path:  %s", event.process_path.c_str());
                        ImGui::Text("Duration: %s", FormatDuration(duration).c_str());
                        ImGui::Text("Title Changes: %d", (int)event.title_changes.size());
                        
                        if (!event.title_changes.empty())
                        {
                            ImGui::Spacing();
                            ImGui::Indent();
                            for (const auto& tc : event.title_changes)
                            {
                                std::string tc_str = FormatTime(tc.timestamp) + " " + tc.title;
                                ImGui::BulletText("%s", tc_str.c_str());
                            }
                            ImGui::Unindent();
                        }
                        
                        ImGui::TreePop();
                    }
                    
                    ImGui::PopID();
                }
            }
            ImGui::EndChild();
            
            // Statistics
            ImGui::Spacing();
            ImGui::Separator();
            
            // Calculate statistics
            long long total_duration = session.end_timestamp - session.start_timestamp;
            std::string most_used = "N/A";
            long long max_duration = 0;
            
            for (int i = 0; i < session.window_focus.size(); i++)
            {
                long long dur = 0;
                if (i < session.window_focus.size() - 1) {
                    dur = session.window_focus[i + 1].focus_timestamp - session.window_focus[i].focus_timestamp;
                } else {
                    dur = session.end_timestamp - session.window_focus[i].focus_timestamp;
                }
                
                if (dur > max_duration) {
                    max_duration = dur;
                    most_used = session.window_focus[i].process_name;
                }
            }
            
            ImGui::Text("Session Statistics:");
            ImGui::SameLine();
            ImGui::Text("Total Time: %s", FormatDuration(total_duration).c_str());
            ImGui::SameLine();
            ImGui::Text(" | Most Used: %s (%s)", most_used.c_str(), FormatDuration(max_duration).c_str());
        }
        else
        {
            ImGui::Text("Select a session to view details");
        }
        
        // Overall statistics
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Overall Statistics: Total Sessions: %d", (int)g_sessions.size());

        ImGui::End();

        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.45f, 0.55f, 0.60f, 1.00f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions for DirectX
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        if (g_pd3dDevice != nullptr)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

