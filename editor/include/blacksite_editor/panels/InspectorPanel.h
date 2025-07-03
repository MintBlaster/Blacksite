#pragma once
#include <blacksite/core/Entity.h>

namespace BlacksiteEditor {

class EditorCore;

class InspectorPanel {
  public:
    InspectorPanel(EditorCore* editorCore);
    ~InspectorPanel() = default;

    void Update(float deltaTime);
    void Render();

  private:
    EditorCore* m_editorCore;
    bool m_isOpen = true;

    void RenderEntityInspector(Blacksite::Entity* entity);
    void RenderTransformComponent(Blacksite::Entity* entity);
    void RenderPhysicsComponent(Blacksite::Entity* entity);
    void RenderRenderComponent(Blacksite::Entity* entity);
    void RenderComponentButtons(Blacksite::Entity* entity);

    void RenderEntityActions(Blacksite::Entity* entity);

    bool DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f,
                         float columnWidth = 100.0f);
    bool DrawColorControl(const std::string& label, glm::vec3& color);
};

}  // namespace BlacksiteEditor
