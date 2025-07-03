#pragma once
#include <functional>
#include "blacksite/core/Entity.h"

namespace BlacksiteEditor {

class EditorCore;

class SceneHierarchyPanel {
public:
    SceneHierarchyPanel(EditorCore* editorCore);
    ~SceneHierarchyPanel() = default;

    void Update(float deltaTime);
    void Render();

    // Callbacks
    using EntitySelectedCallback = std::function<void(int entityId)>;
    void SetEntitySelectedCallback(EntitySelectedCallback callback) { m_entitySelectedCallback = callback; }

private:
    EditorCore* m_editorCore;
    EntitySelectedCallback m_entitySelectedCallback;

    // UI state
    bool m_isOpen = true;
    int m_contextMenuEntityId = -1;

    void RenderEntityNode(const Blacksite::Entity& entity);
    void RenderContextMenu();
    void HandleEntitySelection(int entityId);
    void RenderEntityActions(Blacksite::Entity* entity);

};

}  // namespace BlacksiteEditor
