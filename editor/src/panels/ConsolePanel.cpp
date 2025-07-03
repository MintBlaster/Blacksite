#include "blacksite_editor/panels/ConsolePanel.h"
#include "blacksite_editor/core/EditorCore.h"
#include <blacksite/scene/Scene.h>
#include <blacksite/core/EntitySystem.h>
#include <imgui.h>
#include <cstring>

namespace BlacksiteEditor {

ConsolePanel::ConsolePanel(EditorCore* editorCore) : m_editorCore(editorCore) {
    AddLogEntry("Blacksite Editor Console initialized");
    AddLogEntry("Type 'help' for available commands");
}

void ConsolePanel::Update(float deltaTime) {
    // Simulate periodic log entries for testing
    static float timer = 0.0f;
    timer += deltaTime;

    if (timer > 10.0f) {
        AddLogEntry("System heartbeat - all systems operational");
        timer = 0.0f;
    }
}

void ConsolePanel::Render() {
    if (!m_isOpen) return;

    if (ImGui::Begin("Console", &m_isOpen)) {
        // Console controls
        if (ImGui::Button("Clear")) {
            m_logEntries.clear();
        }
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_autoScroll);

        // Log level filters
        ImGui::SameLine();
        static bool showInfo = true, showWarning = true, showError = true;
        ImGui::Checkbox("Info", &showInfo);
        ImGui::SameLine();
        ImGui::Checkbox("Warning", &showWarning);
        ImGui::SameLine();
        ImGui::Checkbox("Error", &showError);

        ImGui::Separator();

        RenderLogDisplay();
        RenderCommandInput();
    }
    ImGui::End();
}

void ConsolePanel::AddLogEntry(const std::string& message, int level) {
    static float time = 0.0f;
    time += 0.016f; // Approximate time increment

    m_logEntries.push_back({message, level, time});

    // Limit log entries to prevent memory issues
    if (m_logEntries.size() > 1000) {
        m_logEntries.erase(m_logEntries.begin());
    }
}

void ConsolePanel::ProcessCommand(const std::string& command) {
    AddLogEntry("> " + command);

    if (command == "help") {
        AddLogEntry("Available commands:");
        AddLogEntry("  help - Show this help message");
        AddLogEntry("  clear - Clear console output");
        AddLogEntry("  create <type> - Create entity (cube, sphere, plane)");
        AddLogEntry("  delete <id> - Delete entity by ID");
        AddLogEntry("  list - List all entities in scene");
        AddLogEntry("  select <id> - Select entity by ID");
        AddLogEntry("  camera <x> <y> <z> - Set camera position");
        AddLogEntry("  info - Show engine information");
    }
    else if (command == "clear") {
        m_logEntries.clear();
        AddLogEntry("Console cleared");
    }
    else if (command.substr(0, 6) == "create") {
        if (command.length() > 7) {
            std::string type = command.substr(7);
            if (type == "cube" || type == "sphere" || type == "plane") {
                int id = m_editorCore->CreateEntity(type);
                if (id >= 0) {
                    AddLogEntry("Created " + type + " with ID: " + std::to_string(id));
                } else {
                    AddLogEntry("Failed to create " + type, 2);
                }
            } else {
                AddLogEntry("Unknown entity type: " + type + ". Use: cube, sphere, or plane", 1);
            }
        } else {
            AddLogEntry("Usage: create <type>", 1);
        }
    }
    else if (command.substr(0, 6) == "delete") {
        if (command.length() > 7) {
            try {
                int id = std::stoi(command.substr(7));
                m_editorCore->DeleteEntity(id);
                AddLogEntry("Deleted entity ID: " + std::to_string(id));
            } catch (...) {
                AddLogEntry("Invalid entity ID", 2);
            }
        } else {
            AddLogEntry("Usage: delete <id>", 1);
        }
    }
    else if (command.substr(0, 6) == "select") {
        if (command.length() > 7) {
            try {
                int id = std::stoi(command.substr(7));
                m_editorCore->SetSelectedEntity(id);
                AddLogEntry("Selected entity ID: " + std::to_string(id));
            } catch (...) {
                AddLogEntry("Invalid entity ID", 2);
            }
        } else {
            AddLogEntry("Usage: select <id>", 1);
        }
    }
    else if (command == "list") {
        auto* scene = m_editorCore->GetActiveScene();
        if (scene) {
            auto* entitySystem = scene->GetEntitySystem();
            if (entitySystem) {
                const auto& entities = entitySystem->GetEntities();
                AddLogEntry("Entities in scene:");
                int count = 0;
                for (const auto& entity : entities) {
                    if (entity.active) {
                        std::string shapeStr = (entity.shape == Blacksite::Entity::CUBE) ? "Cube" :
                                             (entity.shape == Blacksite::Entity::SPHERE) ? "Sphere" : "Plane";
                        std::string info = "  [" + std::to_string(entity.id) + "] " +
                                         entity.name + " (" + shapeStr + ")";
                        AddLogEntry(info);
                        count++;
                    }
                }
                AddLogEntry("Total: " + std::to_string(count) + " entities");
            }
        } else {
            AddLogEntry("No active scene", 1);
        }
    }
    else if (command == "info") {
        AddLogEntry("Blacksite Engine Information:");
        AddLogEntry("  Version: 1.0.0");
        AddLogEntry("  Physics: Jolt Physics");
        AddLogEntry("  Renderer: OpenGL");
        auto* scene = m_editorCore->GetActiveScene();
        if (scene) {
            AddLogEntry("  Active Scene: " + scene->GetName());
        }
    }
    else if (command.empty()) {
        // Do nothing for empty command
    }
    else {
        AddLogEntry("Unknown command: '" + command + "'. Type 'help' for available commands", 1);
    }
}

void ConsolePanel::RenderLogDisplay() {
    const float footer_height = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height), false, ImGuiWindowFlags_HorizontalScrollbar);

    for (const auto& entry : m_logEntries) {
        ImVec4 color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f); // Default info color

        switch (entry.level) {
            case 1: // Warning
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
                break;
            case 2: // Error
                color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
                break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextWrapped("[%.2f] %s", entry.timestamp, entry.message.c_str());
        ImGui::PopStyleColor();
    }

    if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
}

void ConsolePanel::RenderCommandInput() {
    ImGui::Separator();

    // Command input
    ImGui::SetNextItemWidth(-1);
    bool enterPressed = ImGui::InputText("##CommandInput", m_commandBuffer, sizeof(m_commandBuffer),
                                        ImGuiInputTextFlags_EnterReturnsTrue);

    if (enterPressed && strlen(m_commandBuffer) > 0) {
        ProcessCommand(m_commandBuffer);
        m_commandBuffer[0] = '\0';
        ImGui::SetKeyboardFocusHere(-1); // Keep focus on input
    }

    // Auto-focus on console input when window is focused
    if (ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive()) {
        ImGui::SetKeyboardFocusHere(-1);
    }
}

}  // namespace BlacksiteEditor
