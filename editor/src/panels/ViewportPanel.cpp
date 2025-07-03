#include "blacksite_editor/panels/ViewportPanel.h"
#include <blacksite/core/InputSystem.h>
#include <blacksite/scene/Scene.h>
#include <imgui.h>
#include "blacksite_editor/core/EditorCore.h"

namespace BlacksiteEditor {

ViewportPanel::ViewportPanel(EditorCore* editorCore) : m_editorCore(editorCore) {}

void ViewportPanel::Update(float deltaTime) {
    HandleViewportInput();
}

void ViewportPanel::Render() {
    if (!m_isOpen)
        return;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    if (ImGui::Begin("Viewport", &m_isOpen)) {
        m_isFocused = ImGui::IsWindowFocused();
        m_isHovered = ImGui::IsWindowHovered();

        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();

        // Ensure minimum size
        viewportPanelSize.x = std::max(viewportPanelSize.x, 64.0f);
        viewportPanelSize.y = std::max(viewportPanelSize.y, 64.0f);

        m_viewportSize = {viewportPanelSize.x, viewportPanelSize.y};

        // Viewport content area
        RenderViewportContent();

        // Overlay information
        RenderViewportOverlay();
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void ViewportPanel::HandleViewportInput() {
    if (!m_isFocused && !m_isHovered)
        return;

    auto* input = m_editorCore->GetEngine()->GetInputSystem();
    auto* scene = m_editorCore->GetActiveScene();
    if (!input || !scene)
        return;

    float sensitivity = 0.1f;
    float speed = 5.0f;
    float deltaTime = 1.0f / 60.0f;  // Approximate

    // Mouse look (only when right mouse button is held)
    if (input->MouseDown(Blacksite::Mouse::Right)) {
        glm::vec2 mouseDelta = input->MouseDelta();
        if (glm::length(mouseDelta) > 0.1f && !m_firstMouse) {
            m_yaw += mouseDelta.x * sensitivity;
            m_pitch -= mouseDelta.y * sensitivity;
            m_pitch = glm::clamp(m_pitch, -89.0f, 89.0f);
        } else if (m_firstMouse) {
            m_firstMouse = false;
        }
    } else {
        m_firstMouse = true;
    }

    // Calculate camera vectors
    glm::vec3 forward;
    forward.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    forward.y = sin(glm::radians(m_pitch));
    forward.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    forward = normalize(forward);

    glm::vec3 right = normalize(cross(forward, glm::vec3(0, 1, 0)));
    glm::vec3 up = cross(right, forward);

    // Movement
    glm::vec3 movement = input->Movement3D();

    m_cameraPos += right * movement.x * speed * deltaTime;
    m_cameraPos += up * movement.y * speed * deltaTime;
    m_cameraPos += forward * movement.z * speed * deltaTime;

    // Update scene camera
    scene->SetCameraPosition(m_cameraPos);
    scene->SetCameraTarget(m_cameraPos + forward);
}

void ViewportPanel::RenderViewportContent() {
    auto* renderer = m_editorCore->GetEngine()->GetRenderer();
    auto* scene = m_editorCore->GetActiveScene();

    if (!renderer || !scene) {
        ImGui::Text("No active scene to render");
        return;
    }

    // Get the post-process manager
    auto* postProcessManager = renderer->GetPostProcessManager();
    if (!postProcessManager || !postProcessManager->IsInitialized()) {
        ImGui::Text("Post-process manager not initialized");
        return;
    }

    // Update camera aspect ratio to match viewport
    if (m_viewportSize.x > 0 && m_viewportSize.y > 0) {
        // float aspectRatio = m_viewportSize.x / m_viewportSize.y;
        // Update your scene's camera aspect ratio here
        // scene->GetCameraSystem()->SetAspectRatio(aspectRatio);
    }

    // Resize post-process buffers if viewport size changed
    static glm::vec2 lastViewportSize = {0, 0};
    if (m_viewportSize.x != lastViewportSize.x || m_viewportSize.y != lastViewportSize.y) {
        postProcessManager->OnWindowResize((int)m_viewportSize.x, (int)m_viewportSize.y);
        lastViewportSize = m_viewportSize;
    }

    // Render the scene to the post-process framebuffer
    renderer->BeginFrame();
    scene->Render(renderer);
    renderer->EndFrame();

    // Get the rendered scene texture
    GLuint sceneTexture = postProcessManager->GetSceneTexture();

    if (sceneTexture != 0) {
        // Convert OpenGL texture ID to ImTextureID (ImTextureRef is not standard ImGui)
        ImTextureID textureID = (ImTextureID)(intptr_t)sceneTexture;

        // Display the texture using the ImGui::Image() function
        ImGui::Image(textureID, ImVec2(m_viewportSize.x, m_viewportSize.y),
                    ImVec2(0, 1),    // uv0 - bottom-left in OpenGL coordinates
                    ImVec2(1, 0));   // uv1 - top-right (flipped Y for OpenGL)
    } else {
        ImGui::Text("Failed to get scene texture (ID: %u)", sceneTexture);
    }
}


void ViewportPanel::RenderViewportOverlay() {
    // Overlay controls and information
    ImGui::SetCursorPos(ImVec2(10, m_viewportSize.y - 120));
    ImGui::BeginChild("ViewportOverlay", ImVec2(250, 110), true, ImGuiWindowFlags_NoScrollbar);

    ImGui::Text("Camera Controls:");
    ImGui::Text("Right Click + Drag: Look around");
    ImGui::Text("WASD: Move horizontally");
    ImGui::Text("QE: Move up/down");
    ImGui::Text("Shift: Move faster");

    ImGui::Separator();

    // Quick camera controls
    if (ImGui::Button("Reset Camera")) {
        m_cameraPos = {0, 5, 10};
        m_pitch = -15.0f;
        m_yaw = -90.0f;
        m_firstMouse = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Top View")) {
        m_cameraPos = {0, 20, 0};
        m_pitch = -90.0f;
        m_yaw = -90.0f;
    }

    ImGui::EndChild();
}

}  // namespace BlacksiteEditor
