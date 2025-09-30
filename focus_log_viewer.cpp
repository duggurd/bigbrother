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
#include <shellapi.h>
#include <map>
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

// Icon cache: maps process path to DirectX texture
std::map<std::string, ID3D11ShaderResourceView*> g_iconCache;

// Helper function to get user data path
std::string GetUserDataPath() {
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        return std::string(path) + "\\BigBrother\\focus_log.json";
    }
    return "focus_log.json";
}

// Helper function to convert HICON to DirectX 11 texture
ID3D11ShaderResourceView* CreateTextureFromIcon(HICON hIcon) {
    if (!hIcon) return nullptr;
    
    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo)) {
        return nullptr;
    }
    
    BITMAP bmp;
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmp);
    
    int width = bmp.bmWidth;
    int height = bmp.bmHeight;
    
    // Create a DIB section to get the icon bitmap data
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    
    void* bits = nullptr;
    HDC hdc = GetDC(NULL);
    HBITMAP hDIB = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    
    if (hDIB && bits) {
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hDIB);
        
        // Draw icon into DIB
        DrawIconEx(hdcMem, 0, 0, hIcon, width, height, 0, NULL, DI_NORMAL);
        
        SelectObject(hdcMem, hOldBitmap);
        DeleteDC(hdcMem);
        
        // Convert BGRA to RGBA (Windows uses BGRA, DirectX expects RGBA)
        unsigned char* pixels = (unsigned char*)bits;
        int totalPixels = width * height;
        for (int i = 0; i < totalPixels; i++) {
            int idx = i * 4;
            unsigned char b = pixels[idx + 0];
            unsigned char r = pixels[idx + 2];
            pixels[idx + 0] = r;  // Red
            pixels[idx + 2] = b;  // Blue
            // Green (idx+1) and Alpha (idx+3) stay the same
        }
        
        // Create DirectX texture
        D3D11_TEXTURE2D_DESC desc = {};
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        
        D3D11_SUBRESOURCE_DATA initData = {};
        initData.pSysMem = bits;
        initData.SysMemPitch = width * 4;
        
        ID3D11Texture2D* pTexture = nullptr;
        HRESULT hr = g_pd3dDevice->CreateTexture2D(&desc, &initData, &pTexture);
        
        ID3D11ShaderResourceView* pSRV = nullptr;
        if (SUCCEEDED(hr)) {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = desc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            
            hr = g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
            pTexture->Release();
        }
        
        DeleteObject(hDIB);
        ReleaseDC(NULL, hdc);
        
        return pSRV;
    }
    
    ReleaseDC(NULL, hdc);
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    
    return nullptr;
}

// Extract icon from executable and convert to DirectX texture
ID3D11ShaderResourceView* GetApplicationIcon(const std::string& exePath) {
    // Check cache first
    auto it = g_iconCache.find(exePath);
    if (it != g_iconCache.end()) {
        return it->second;
    }
    
    // Extract icon from executable
    HICON hIcon = nullptr;
    
    // Try to get the icon using SHGetFileInfo first (faster)
    SHFILEINFOA sfi = {};
    if (SHGetFileInfoA(exePath.c_str(), 0, &sfi, sizeof(sfi), SHGFI_ICON | SHGFI_SMALLICON)) {
        hIcon = sfi.hIcon;
    }
    
    // If that didn't work, try ExtractIcon
    if (!hIcon) {
        hIcon = ExtractIconA(GetModuleHandle(NULL), exePath.c_str(), 0);
        if (hIcon == (HICON)1 || !hIcon) { // ExtractIcon returns 1 if no icon
            hIcon = nullptr;
        }
    }
    
    ID3D11ShaderResourceView* texture = nullptr;
    if (hIcon) {
        texture = CreateTextureFromIcon(hIcon);
        DestroyIcon(hIcon);
    }
    
    // Cache the result (even if null, to avoid repeated failures)
    g_iconCache[exePath] = texture;
    
    return texture;
}

// Clean up icon cache
void CleanupIconCache() {
    for (auto& pair : g_iconCache) {
        if (pair.second) {
            pair.second->Release();
        }
    }
    g_iconCache.clear();
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
                        
                        // Backward compatibility: if old data has window_title in focus event,
                        // move it to the first title change
                        if (focusJson.contains("window_title") && !focusJson["window_title"].is_null()) {
                            std::string old_title = focusJson.value("window_title", "");
                            if (!old_title.empty() && event.title_changes.empty()) {
                                TitleChange tc;
                                tc.timestamp = event.focus_timestamp;
                                tc.title = old_title;
                                event.title_changes.insert(event.title_changes.begin(), tc);
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
                
                // Session header with timestamps
                std::string start_time = FormatTime(session.start_timestamp);
                std::string end_time = FormatTime(session.end_timestamp);
                std::string header = "Session " + std::to_string(i + 1) + 
                                    "  (" + start_time + " - " + end_time + ")";
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
                    
                    // Event header with icon and bold text
                    std::string time_str = FormatTime(event.focus_timestamp);
                    std::string header = time_str + " - " + event.process_name + " (" + FormatDuration(duration) + ")";
                    
                    // Get application icon
                    ID3D11ShaderResourceView* icon = GetApplicationIcon(event.process_path);
                    
                    // Display icon if available
                    if (icon) {
                        ImGui::Image((void*)icon, ImVec2(16, 16));
                        ImGui::SameLine();
                    }
                    
                    // Make the header bold and more visible
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.6f, 1.0f)); // Light yellow color
                    bool node_open = ImGui::TreeNode(header.c_str());
                    ImGui::PopStyleColor();
                    
                    if (node_open)
                    {
                        ImGui::Text("Path:  %s", event.process_path.c_str());
                        ImGui::Text("Duration: %s", FormatDuration(duration).c_str());
                        
                        if (!event.title_changes.empty())
                        {
                            ImGui::Spacing();
                            ImGui::Text("Window Titles:");
                            ImGui::Indent();
                            for (const auto& tc : event.title_changes)
                            {
                                std::string tc_str = FormatTime(tc.timestamp) + " " + tc.title;
                                ImGui::BulletText("%s", tc_str.c_str());
                            }
                            ImGui::Unindent();
                        }
                        else
                        {
                            ImGui::Text("Window Titles: None recorded");
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
    CleanupIconCache();
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

