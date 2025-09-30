#pragma once

#include <d3d11.h>
#include <string>
#include <map>

namespace bigbrother {
namespace viewer {

/**
 * @brief Manages application icons
 * 
 * Extracts icons from executable files, converts them to DirectX textures,
 * and caches them for efficient reuse.
 */
class IconManager {
public:
    IconManager(ID3D11Device* device);
    ~IconManager();

    /**
     * @brief Get icon texture for an executable
     * @param exePath Full path to executable file
     * @return DirectX texture, or nullptr if icon couldn't be loaded
     */
    ID3D11ShaderResourceView* GetIcon(const std::string& exePath);

    /**
     * @brief Clear all cached icons and free memory
     */
    void ClearCache();

private:
    ID3D11Device* m_device;
    std::map<std::string, ID3D11ShaderResourceView*> m_iconCache;

    // Convert Windows HICON to DirectX texture
    ID3D11ShaderResourceView* CreateTextureFromIcon(HICON hIcon);
};

} // namespace viewer
} // namespace bigbrother
