#include "blacksite_editor/panels/AssetBrowserPanel.h"
#include "blacksite_editor/core/EditorCore.h"
#include <imgui.h>
#include <algorithm>

namespace BlacksiteEditor {

AssetBrowserPanel::AssetBrowserPanel(EditorCore* editorCore) : m_editorCore(editorCore) {
    RefreshCurrentDirectory();
}

void AssetBrowserPanel::Update(float deltaTime) {
    // Update logic if needed
}

void AssetBrowserPanel::Render() {
    if (!m_isOpen) return;

    if (ImGui::Begin("Asset Browser", &m_isOpen)) {
        RenderDirectoryNavigation();
        ImGui::Separator();
        RenderAssetGrid();
        RenderAssetContextMenu();
    }
    ImGui::End();
}

void AssetBrowserPanel::RefreshCurrentDirectory() {
    m_currentItems.clear();

    try {
        // Add parent directory option (if not at root)
        if (m_currentPath != "assets/" && m_currentPath.find("assets/") == 0) {
            AssetItem parentItem;
            parentItem.name = "..";
            parentItem.path = m_currentPath;
            parentItem.isDirectory = true;
            m_currentItems.push_back(parentItem);
        }

        // For now, add some mock assets since we don't have filesystem scanning
        // In a real implementation, you'd scan the actual directory

        // Mock directories
        if (m_currentPath == "assets/") {
            m_currentItems.push_back({"textures", "assets/textures/", "", true});
            m_currentItems.push_back({"models", "assets/models/", "", true});
            m_currentItems.push_back({"shaders", "assets/shaders/", "", true});
            m_currentItems.push_back({"audio", "assets/audio/", "", true});
            m_currentItems.push_back({"scenes", "assets/scenes/", "", true});
        }

        // Mock files based on directory
        if (m_currentPath == "assets/textures/") {
            m_currentItems.push_back({"default.png", "assets/textures/default.png", ".png", false});
            m_currentItems.push_back({"wood.jpg", "assets/textures/wood.jpg", ".jpg", false});
            m_currentItems.push_back({"metal.tga", "assets/textures/metal.tga", ".tga", false});
        } else if (m_currentPath == "assets/models/") {
            m_currentItems.push_back({"cube.obj", "assets/models/cube.obj", ".obj", false});
            m_currentItems.push_back({"sphere.fbx", "assets/models/sphere.fbx", ".fbx", false});
            m_currentItems.push_back({"character.dae", "assets/models/character.dae", ".dae", false});
        } else if (m_currentPath == "assets/shaders/") {
            m_currentItems.push_back({"basic.vert", "assets/shaders/basic.vert", ".vert", false});
            m_currentItems.push_back({"basic.frag", "assets/shaders/basic.frag", ".frag", false});
            m_currentItems.push_back({"unlit.glsl", "assets/shaders/unlit.glsl", ".glsl", false});
        } else if (m_currentPath == "assets/scenes/") {
            m_currentItems.push_back({"test_scene.bscn", "assets/scenes/test_scene.bscn", ".bscn", false});
            m_currentItems.push_back({"demo.bscn", "assets/scenes/demo.bscn", ".bscn", false});
        }

    } catch (...) {
        // Handle filesystem errors
        AssetItem errorItem;
        errorItem.name = "Error reading directory";
        errorItem.isDirectory = false;
        m_currentItems.push_back(errorItem);
    }
}

void AssetBrowserPanel::NavigateToPath(const std::string& path) {
    m_currentPath = path;
    m_selectedItem = -1;
    RefreshCurrentDirectory();
}

void AssetBrowserPanel::RenderDirectoryNavigation() {
    // Current path display
    ImGui::Text("Path: %s", m_currentPath.c_str());

    // Navigation buttons
    if (ImGui::Button("Assets Root")) {
        NavigateToPath("assets/");
    }

    ImGui::SameLine();
    if (ImGui::Button("Refresh")) {
        RefreshCurrentDirectory();
    }

    ImGui::SameLine();
    if (ImGui::Button("Up") && m_currentPath != "assets/") {
        // Navigate to parent directory
        size_t lastSlash = m_currentPath.find_last_of('/', m_currentPath.length() - 2);
        if (lastSlash != std::string::npos) {
            NavigateToPath(m_currentPath.substr(0, lastSlash + 1));
        }
    }
}

void AssetBrowserPanel::RenderAssetGrid() {
    // Asset grid
    float panelWidth = ImGui::GetContentRegionAvail().x;
    float itemSize = 80.0f;
    int columns = std::max(1, (int)(panelWidth / (itemSize + ImGui::GetStyle().ItemSpacing.x)));

    ImGui::Columns(columns, nullptr, false);

    for (int i = 0; i < m_currentItems.size(); ++i) {
        const auto& item = m_currentItems[i];

        ImGui::PushID(i);

        // Item button
        bool isSelected = (m_selectedItem == i);
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
        }

        // Choose icon based on type
        const char* icon = "📄"; // Default file
        if (item.isDirectory) {
            icon = "📁";
        } else if (IsImageFile(item.extension)) {
            icon = "🖼️";
        } else if (IsModelFile(item.extension)) {
            icon = "🎲";
        } else if (IsAudioFile(item.extension)) {
            icon = "🔊";
        } else if (item.extension == ".bscn") {
            icon = "🎬";
        } else if (item.extension == ".vert" || item.extension == ".frag" || item.extension == ".glsl") {
            icon = "⚡";
        }

        if (ImGui::Button((std::string(icon) + "\n" + item.name).c_str(), ImVec2(itemSize, itemSize))) {
            m_selectedItem = i;

            if (item.isDirectory) {
                if (item.name == "..") {
                    // Navigate up
                    size_t lastSlash = m_currentPath.find_last_of('/', m_currentPath.length() - 2);
                    if (lastSlash != std::string::npos) {
                        NavigateToPath(m_currentPath.substr(0, lastSlash + 1));
                    }
                } else {
                    // Navigate into directory
                    NavigateToPath(item.path);
                }
            }
        }

        if (isSelected) {
            ImGui::PopStyleColor();
        }

        // Tooltip with full path
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", item.path.c_str());
        }

        ImGui::PopID();
        ImGui::NextColumn();
    }

    ImGui::Columns(1);

    // Selected item info
    if (m_selectedItem >= 0 && m_selectedItem < m_currentItems.size()) {
        const auto& selected = m_currentItems[m_selectedItem];
        ImGui::Separator();
        ImGui::Text("Selected: %s", selected.name.c_str());
        ImGui::Text("Path: %s", selected.path.c_str());
        if (!selected.isDirectory) {
            ImGui::Text("Type: %s", selected.extension.c_str());
        }
    }
}

void AssetBrowserPanel::RenderAssetContextMenu() {
    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::MenuItem("Create Folder")) {
            // TODO: Implement folder creation
        }
        if (ImGui::MenuItem("Import Asset")) {
            // TODO: Implement asset import
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Refresh")) {
            RefreshCurrentDirectory();
        }
        ImGui::EndPopup();
    }
}

bool AssetBrowserPanel::IsImageFile(const std::string& extension) {
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
           extension == ".tga" || extension == ".bmp" || extension == ".dds";
}

bool AssetBrowserPanel::IsModelFile(const std::string& extension) {
    return extension == ".obj" || extension == ".fbx" || extension == ".dae" ||
           extension == ".gltf" || extension == ".glb" || extension == ".3ds";
}

bool AssetBrowserPanel::IsAudioFile(const std::string& extension) {
    return extension == ".wav" || extension == ".mp3" || extension == ".ogg" ||
           extension == ".flac" || extension == ".aac";
}

}  // namespace BlacksiteEditor
