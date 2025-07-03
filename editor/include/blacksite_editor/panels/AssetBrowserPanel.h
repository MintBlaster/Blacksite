#pragma once
#include <string>
#include <vector>
#include <filesystem>

namespace BlacksiteEditor {

class EditorCore;

struct AssetItem {
    std::string name;
    std::string path;
    std::string extension;
    bool isDirectory;
};

class AssetBrowserPanel {
public:
    AssetBrowserPanel(EditorCore* editorCore);
    ~AssetBrowserPanel() = default;

    void Update(float deltaTime);
    void Render();

private:
    EditorCore* m_editorCore;
    bool m_isOpen = true;

    std::string m_currentPath = "assets/";
    std::vector<AssetItem> m_currentItems;
    int m_selectedItem = -1;

    void RefreshCurrentDirectory();
    void NavigateToPath(const std::string& path);
    void RenderDirectoryNavigation();
    void RenderAssetGrid();
    void RenderAssetContextMenu();
    bool IsImageFile(const std::string& extension);
    bool IsModelFile(const std::string& extension);
    bool IsAudioFile(const std::string& extension);
};

}  // namespace BlacksiteEditor
