#include "icon_manager.h"
#include <windows.h>
#include <shellapi.h>

namespace bigbrother {
namespace viewer {

IconManager::IconManager(ID3D11Device* device)
    : m_device(device)
{
}

IconManager::~IconManager() {
    ClearCache();
}

ID3D11ShaderResourceView* IconManager::GetIcon(const std::string& exePath) {
    // Check cache first
    auto it = m_iconCache.find(exePath);
    if (it != m_iconCache.end()) {
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
    m_iconCache[exePath] = texture;
    
    return texture;
}

void IconManager::ClearCache() {
    for (auto& pair : m_iconCache) {
        if (pair.second) {
            pair.second->Release();
        }
    }
    m_iconCache.clear();
}

ID3D11ShaderResourceView* IconManager::CreateTextureFromIcon(HICON hIcon) {
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
        HRESULT hr = m_device->CreateTexture2D(&desc, &initData, &pTexture);
        
        ID3D11ShaderResourceView* pSRV = nullptr;
        if (SUCCEEDED(hr)) {
            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = desc.Format;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
            srvDesc.Texture2D.MipLevels = 1;
            
            hr = m_device->CreateShaderResourceView(pTexture, &srvDesc, &pSRV);
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

} // namespace viewer
} // namespace bigbrother
