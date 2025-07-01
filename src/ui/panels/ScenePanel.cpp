#include "blacksite/ui/panels/ScenePanel.h"
#include "blacksite/core/Engine.h"
#include "blacksite/core/Logger.h"

namespace Blacksite::UI {

// SceneHierarchyPanel Implementation
SceneHierarchyPanel::SceneHierarchyPanel() 
    : Panel("scene_hierarchy", "Scene Hierarchy") {
}

void SceneHierarchyPanel::Render(UISystem& ui) {
    if (!IsVisible()) return;

    // Get engine reference from somewhere - we'll need to pass this in
    // For now, let's assume we can get it through a global or singleton
    // This is a design choice we'll need to address
    
    bool open = true;
    ui.BeginWindow(GetTitle(), &open);
    if (!open) {
        SetVisible(false);
        ui.EndWindow();
        return;
    }

    // Search filter
    ui.Text("Search:");
    ui.SameLine();
    if (ui.InputText("##search", m_searchFilter)) {
        // Filter updated
    }
    
    ui.Checkbox("Show Inactive", m_showInactiveEntities);
    ui.Separator();

    // Entity list header
    ui.Text("Entities:");
    ui.Separator();

    // We'll need a way to access the engine here
    // For now, let's create a placeholder structure
    ui.Text("Entity list will go here");
    ui.Text("Selected: " + (m_selectedEntityId >= 0 ? std::to_string(m_selectedEntityId) : "None"));
    
    // Sample entity nodes
    for (int i = 0; i < 5; ++i) {
        std::string entityName = "Entity " + std::to_string(i);
        bool isSelected = (m_selectedEntityId == i);
        
        if (ui.Button(entityName.c_str())) {
            m_selectedEntityId = i;
            BS_DEBUG_F(LogCategory::EDITOR, "Selected entity %d", i);
        }
    }

    ui.EndWindow();
}

void SceneHierarchyPanel::RenderEntityNode(UISystem& ui, Engine& engine, int entityId) {
    // Implementation for rendering individual entity nodes
    const auto& entities = engine.GetEntities();
    if (entityId >= 0 && entityId < static_cast<int>(entities.size())) {
        const auto& entity = entities[entityId];
        
        if (!entity.active && !m_showInactiveEntities) {
            return;
        }
        
        std::string icon = "[CUBE]";
        if (entity.shape == Entity::SPHERE) icon = "[SPHERE]";
        if (entity.shape == Entity::PLANE) icon = "[PLANE]";
        
        std::string label = icon + " Entity " + std::to_string(entityId);
        if (!entity.active) label += " (Inactive)";
        
        bool isSelected = (m_selectedEntityId == entityId);
        if (ui.Button(label.c_str())) {
            m_selectedEntityId = entityId;
        }
    }
}

void SceneHierarchyPanel::RenderContextMenu(UISystem& ui, Engine& engine, int entityId) {
    // Right-click context menu implementation
    // This will need ImGui-specific code for context menus
}

// EntityInspectorPanel Implementation
EntityInspectorPanel::EntityInspectorPanel() 
    : Panel("entity_inspector", "Entity Inspector") {
}

void EntityInspectorPanel::Render(UISystem& ui) {
    if (!IsVisible()) return;

    bool open = true;
    ui.BeginWindow(GetTitle(), &open);
    if (!open) {
        SetVisible(false);
        ui.EndWindow();
        return;
    }

    if (m_selectedEntityId < 0) {
        ui.Text("No entity selected");
        ui.Text("Select an entity from the Scene Hierarchy");
        ui.EndWindow();
        return;
    }

    ui.Text("Entity ID: " + std::to_string(m_selectedEntityId));
    ui.Separator();

    // Transform section
    if (ui.Button("Transform")) {
        // Toggle transform section
    }
    ui.Text("Position: (0, 0, 0)");
    ui.Text("Rotation: (0, 0, 0)");
    ui.Text("Scale: (1, 1, 1)");
    
    ui.Separator();
    
    // Physics section
    if (ui.Button("Physics")) {
        // Toggle physics section
    }
    ui.Text("Mass: 1.0");
    ui.Text("Velocity: (0, 0, 0)");
    
    ui.Separator();
    
    // Rendering section
    if (ui.Button("Rendering")) {
        // Toggle rendering section
    }
    ui.Text("Color: (1, 1, 1)");
    ui.Text("Material: Default");

    ui.EndWindow();
}

void EntityInspectorPanel::RenderTransformSection(UISystem& ui, Engine& engine) {
    // Transform editing implementation
}

void EntityInspectorPanel::RenderPhysicsSection(UISystem& ui, Engine& engine) {
    // Physics properties editing implementation
}

void EntityInspectorPanel::RenderRenderingSection(UISystem& ui, Engine& engine) {
    // Rendering properties editing implementation
}

// ConsolePanel Implementation
ConsolePanel::ConsolePanel() 
    : Panel("console", "Console") {
}

void ConsolePanel::Render(UISystem& ui) {
    if (!IsVisible()) return;

    bool open = true;
    ui.BeginWindow(GetTitle(), &open);
    if (!open) {
        SetVisible(false);
        ui.EndWindow();
        return;
    }

    // Filter controls
    ui.Text("Filters:");
    ui.SameLine();
    ui.Checkbox("Trace", m_showTrace);
    ui.SameLine();
    ui.Checkbox("Debug", m_showDebug);
    ui.SameLine();
    ui.Checkbox("Info", m_showInfo);
    ui.SameLine();
    ui.Checkbox("Warn", m_showWarn);
    ui.SameLine();
    ui.Checkbox("Error", m_showError);
    ui.SameLine();
    ui.Checkbox("Fatal", m_showFatal);
    
    ui.SameLine();
    ui.Checkbox("Auto-scroll", m_autoScroll);
    ui.SameLine();
    ui.Checkbox("Timestamps", m_showTimestamps);
    
    if (ui.Button("Clear")) {
        BS_INFO(LogCategory::EDITOR, "Console cleared");
    }
    
    ui.Separator();

    // Log history area
    RenderLogHistory(ui);
    
    ui.Separator();
    
    // Command input
    RenderCommandInput(ui);

    ui.EndWindow();
}

void ConsolePanel::RenderLogHistory(UISystem& ui) {
    // Get log history from Logger
    const auto& logHistory = Logger::Instance().GetLogHistory();
    
    ui.Text("Log History (" + std::to_string(logHistory.size()) + " entries):");
    
    // For now, show a simple list
    for (size_t i = 0; i < std::min(logHistory.size(), size_t(10)); ++i) {
        const auto& entry = logHistory[logHistory.size() - 1 - i]; // Show newest first
        
        // Apply filters
        bool shouldShow = true;
        switch (entry.level) {
            case LogLevel::TRACE: shouldShow = m_showTrace; break;
            case LogLevel::DEBUG: shouldShow = m_showDebug; break;
            case LogLevel::INFO: shouldShow = m_showInfo; break;
            case LogLevel::WARN: shouldShow = m_showWarn; break;
            case LogLevel::ERROR: shouldShow = m_showError; break;
            case LogLevel::FATAL: shouldShow = m_showFatal; break;
        }
        
        if (!shouldShow) continue;
        
        std::string logLine;
        if (m_showTimestamps) {
            logLine = "[" + entry.timestamp + "] ";
        }
        logLine += entry.message;
        
        // Color based on log level
        if (entry.level == LogLevel::ERROR || entry.level == LogLevel::FATAL) {
            ui.TextColored(glm::vec3(1.0f, 0.3f, 0.3f), logLine);
        } else if (entry.level == LogLevel::WARN) {
            ui.TextColored(glm::vec3(1.0f, 1.0f, 0.3f), logLine);
        } else {
            ui.Text(logLine);
        }
    }
}

void ConsolePanel::RenderCommandInput(UISystem& ui) {
    ui.Text("Command:");
    ui.SameLine();
    
    if (ui.InputText("##command", m_commandInput)) {
        // Command input updated
    }
    
    ui.SameLine();
    if (ui.Button("Execute")) {
        if (!m_commandInput.empty()) {
            ExecuteCommand(m_commandInput);
            m_commandInput.clear();
        }
    }
}

void ConsolePanel::ExecuteCommand(const std::string& command) {
    BS_INFO_F(LogCategory::EDITOR, "Executing command: %s", command.c_str());
    
    if (command == "help") {
        BS_INFO(LogCategory::EDITOR, "Available commands:");
        BS_INFO(LogCategory::EDITOR, "  help - Show this help");
        BS_INFO(LogCategory::EDITOR, "  clear - Clear console");
        BS_INFO(LogCategory::EDITOR, "  spawn_cube - Spawn a test cube");
        BS_INFO(LogCategory::EDITOR, "  spawn_sphere - Spawn a test sphere");
    } else if (command == "clear") {
        Logger::Instance().ClearHistory();
    } else if (command == "spawn_cube") {
        BS_INFO(LogCategory::EDITOR, "Would spawn cube (engine access needed)");
    } else if (command == "spawn_sphere") {
        BS_INFO(LogCategory::EDITOR, "Would spawn sphere (engine access needed)");
    } else {
        BS_WARN_F(LogCategory::EDITOR, "Unknown command: %s", command.c_str());
    }
}

// PerformancePanel Implementation
PerformancePanel::PerformancePanel() 
    : Panel("performance", "Performance") {
    m_frameHistory.reserve(MAX_FRAME_HISTORY);
}

void PerformancePanel::Render(UISystem& ui) {
    if (!IsVisible()) return;

    bool open = true;
    ui.BeginWindow(GetTitle(), &open);
    if (!open) {
        SetVisible(false);
        ui.EndWindow();
        return;
    }

    UpdateMetrics();
    
    ui.Text("Performance Metrics");
    ui.Separator();
    
    ui.Text("FPS: " + std::to_string(static_cast<int>(m_fps)));
    ui.Text("Frame Time: " + std::to_string(m_frameTime * 1000.0f) + " ms");
    
    ui.Separator();
    
    RenderFPSGraph(ui);
    
    ui.Separator();
    
    RenderMemoryInfo(ui);
    
    ui.Separator();
    
    RenderSystemInfo(ui);

    ui.EndWindow();
}

void PerformancePanel::UpdateMetrics() {
    // Calculate FPS and frame time
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    m_frameTime = std::chrono::duration<float>(currentTime - lastTime).count();
    lastTime = currentTime;
    
    if (m_frameTime > 0.0f) {
        m_fps = 1.0f / m_frameTime;
    }
    
    // Update frame history
    m_frameHistory.push_back(m_frameTime);
    if (m_frameHistory.size() > MAX_FRAME_HISTORY) {
        m_frameHistory.erase(m_frameHistory.begin());
    }
}

void PerformancePanel::RenderFPSGraph(UISystem& ui) {
    ui.Text("Frame Time Graph");
    ui.Text("(Last " + std::to_string(m_frameHistory.size()) + " frames)");
    
    if (!m_frameHistory.empty()) {
        float minTime = *std::min_element(m_frameHistory.begin(), m_frameHistory.end());
        float maxTime = *std::max_element(m_frameHistory.begin(), m_frameHistory.end());
        ui.Text("Min: " + std::to_string(minTime * 1000.0f) + " ms");
        ui.Text("Max: " + std::to_string(maxTime * 1000.0f) + " ms");
    }
}

void PerformancePanel::RenderMemoryInfo(UISystem& ui) {
    ui.Text("Memory Information");
    ui.Text("Engine Memory: ~" + std::to_string(sizeof(Engine)) + " bytes");
    ui.Text("Entity Count: (needs engine reference)");
}

void PerformancePanel::RenderSystemInfo(UISystem& ui) {
    ui.Text("System Information");
    ui.Text("OpenGL Renderer: (needs GL query)");
    ui.Text("Platform: Linux");
}

} // namespace Blacksite::UI