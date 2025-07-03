#include "blacksite_editor/panels/PerformancePanel.h"
#include "blacksite_editor/core/EditorCore.h"
#include <blacksite/core/Engine.h>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include <GL/glew.h>

namespace BlacksiteEditor {

PerformancePanel::PerformancePanel(EditorCore* editorCore) : m_editorCore(editorCore) {
    // Initialize frame time history
    for (int i = 0; i < 120; ++i) {
        m_frameTimeHistory[i] = 0.0f;
    }
}

void PerformancePanel::Update(float deltaTime) {
    UpdatePerformanceStats(deltaTime);
}

void PerformancePanel::Render() {
    if (!m_isOpen) return;

    if (ImGui::Begin("Performance", &m_isOpen)) {
        RenderFrameRateInfo();
        ImGui::Separator();
        RenderMemoryInfo();
        ImGui::Separator();
        RenderSystemInfo();
        ImGui::Separator();
        RenderPerformanceControls();
    }
    ImGui::End();
}

void PerformancePanel::UpdatePerformanceStats(float deltaTime) {
    // Update frame time history
    m_frameTimeHistory[m_frameTimeIndex] = deltaTime * 1000.0f; // Convert to milliseconds
    m_frameTimeIndex = (m_frameTimeIndex + 1) % 120;

    // Calculate FPS
    if (deltaTime > 0.0f) {
        m_currentFPS = 1.0f / deltaTime;
    }
}

void PerformancePanel::RenderFrameRateInfo() {
    ImGui::Text("Frame Rate Information");

    float currentFrameTime = m_frameTimeHistory[m_frameTimeIndex > 0 ? m_frameTimeIndex - 1 : 119];

    ImGui::Text("FPS: %.1f", m_currentFPS);
    ImGui::Text("Frame Time: %.2f ms", currentFrameTime);

    // Calculate average frame time
    float avgFrameTime = 0.0f;
    for (int i = 0; i < 120; ++i) {
        avgFrameTime += m_frameTimeHistory[i];
    }
    avgFrameTime /= 120.0f;
    ImGui::Text("Avg Frame Time: %.2f ms", avgFrameTime);

    // Frame time graph
    ImGui::PlotLines("Frame Time (ms)", m_frameTimeHistory, 120, m_frameTimeIndex,
                     nullptr, 0.0f, 50.0f, ImVec2(0, 80));

    // Performance indicators
    if (m_currentFPS >= 60.0f) {
        ImGui::TextColored(ImVec4(0, 1, 0, 1), "Performance: Excellent");
    } else if (m_currentFPS >= 30.0f) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Performance: Good");
    } else {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Performance: Poor");
    }
}

void PerformancePanel::RenderMemoryInfo() {
    ImGui::Text("Memory & Entity Information");

    auto* scene = m_editorCore->GetActiveScene();
    if (scene) {
        auto* entitySystem = scene->GetEntitySystem();
        if (entitySystem) {
            const auto& entities = entitySystem->GetEntities();
            int activeEntities = 0;
            int physicsEntities = 0;

            for (const auto& entity : entities) {
                if (entity.active) {
                    activeEntities++;
                    if (entity.hasPhysics) {
                        physicsEntities++;
                    }
                }
            }

            ImGui::Text("Active Entities: %d", activeEntities);
            ImGui::Text("Total Entities: %zu", entities.size());
            ImGui::Text("Physics Entities: %d", physicsEntities);

            // Memory estimation (rough)
            size_t estimatedMemory = entities.size() * sizeof(Blacksite::Entity);
            ImGui::Text("Est. Entity Memory: %.2f KB", estimatedMemory / 1024.0f);
        }
    } else {
        ImGui::Text("No active scene");
    }
}

void PerformancePanel::RenderSystemInfo() {
    ImGui::Text("System Information");

    auto* engine = m_editorCore->GetEngine();

    // OpenGL information
    const char* glVersion = (const char*)glGetString(GL_VERSION);
    const char* glRenderer = (const char*)glGetString(GL_RENDERER);
    const char* glVendor = (const char*)glGetString(GL_VENDOR);

    ImGui::Text("OpenGL Version: %s", glVersion ? glVersion : "Unknown");
    ImGui::Text("GPU: %s", glRenderer ? glRenderer : "Unknown");
    ImGui::Text("Vendor: %s", glVendor ? glVendor : "Unknown");

    // Engine systems status
    ImGui::Text("Engine Systems:");
    ImGui::Text("  Physics: %s", engine->GetPhysicsSystem() ? "Active" : "Inactive");
    ImGui::Text("  Renderer: %s", engine->GetRenderer() ? "Active" : "Inactive");
    ImGui::Text("  Input: %s", engine->GetInputSystem() ? "Active" : "Inactive");

    // Window information
    GLFWwindow* window = engine->GetGLFWWindow();
    if (window) {
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        ImGui::Text("Window Size: %dx%d", width, height);

        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        ImGui::Text("Framebuffer Size: %dx%d", fbWidth, fbHeight);
    }
}

void PerformancePanel::RenderPerformanceControls() {
    ImGui::Text("Performance Controls");

    // VSync control
    static bool vsync = true;
    if (ImGui::Checkbox("VSync", &vsync)) {
        glfwSwapInterval(vsync ? 1 : 0);
    }

    // Target FPS (for future use)
    static int targetFPS = 60;
    ImGui::SliderInt("Target FPS", &targetFPS, 30, 144);
    ImGui::SameLine();
    ImGui::Text("(Not implemented)");

    // Performance actions
    if (ImGui::Button("Reset Performance Counters")) {
        for (int i = 0; i < 120; ++i) {
            m_frameTimeHistory[i] = 0.0f;
        }
        m_frameTimeIndex = 0;
        m_currentFPS = 0.0f;
    }

    ImGui::SameLine();
    if (ImGui::Button("Force GC")) {
        // Placeholder for garbage collection or memory cleanup
        ImGui::OpenPopup("GC Info");
    }

    if (ImGui::BeginPopup("GC Info")) {
        ImGui::Text("Garbage collection not implemented");
        ImGui::Text("This would clean up unused resources");
        ImGui::EndPopup();
    }

    // Performance warnings
    if (m_currentFPS < 30.0f && m_currentFPS > 0.0f) {
        ImGui::TextColored(ImVec4(1, 0, 0, 1), "Warning: Low frame rate detected!");
        ImGui::Text("Consider reducing scene complexity");
    }
}

}  // namespace BlacksiteEditor
