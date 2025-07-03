#pragma once
#include <glm/glm.hpp>
#include "blacksite/graphics/PostProcessManager.h"

namespace BlacksiteEditor {

class EditorCore;

class ViewportPanel {
public:
    ViewportPanel(EditorCore* editorCore);
    ~ViewportPanel() = default;

    void Update(float deltaTime);
    void Render();

    bool IsHovered() const { return m_isHovered; }
    bool IsFocused() const { return m_isFocused; }
    glm::vec2 GetSize() const { return m_viewportSize; }

private:
    EditorCore* m_editorCore;
    bool m_isOpen = true;
    bool m_isHovered = false;
    bool m_isFocused = false;
    glm::vec2 m_viewportSize = {1280, 720};

    // Camera control state
    glm::vec3 m_cameraPos = {0, 5, 10};
    float m_pitch = -15.0f;
    float m_yaw = -90.0f;
    bool m_firstMouse = true;

    void HandleViewportInput();
    void RenderViewportContent();
    void RenderViewportOverlay();
};

}  // namespace BlacksiteEditor
