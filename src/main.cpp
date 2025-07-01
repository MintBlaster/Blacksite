#include "blacksite/Blacksite.h"
#include "blacksite/ui/panels/ScenePanel.h"
#include <memory>

using namespace Blacksite;

int main() {
    // Initialize logger first
    Logger::Instance().Initialize();
    BS_INFO(LogCategory::CORE, "Blacksite Engine - Editor Mode");
    BS_INFO(LogCategory::CORE, "UI System initialization starting...");

    Engine engine;

    if (!engine.Initialize(1920, 1080, "Blacksite Engine - Editor")) {
        BS_ERROR(LogCategory::CORE, "Engine initialization failed");
        return -1;
    }

    // Get UI system and setup panels
    auto* ui = engine.GetUI();
    if (!ui) {
        BS_ERROR(LogCategory::EDITOR, "UI system not available");
        return -1;
    }

    // Create panel manager and register panels
    UI::PanelManager panelManager;
    
    panelManager.RegisterPanel(std::make_unique<UI::SceneHierarchyPanel>());
    panelManager.RegisterPanel(std::make_unique<UI::EntityInspectorPanel>());
    panelManager.RegisterPanel(std::make_unique<UI::ConsolePanel>());
    panelManager.RegisterPanel(std::make_unique<UI::PerformancePanel>());

    BS_INFO(LogCategory::EDITOR, "Editor panels registered");

    // Create a simple test scene
    BS_INFO(LogCategory::GAME, "Setting up test scene...");

    // Ground plane
    int ground = engine.SpawnPlane({0, -1, 0}, {20, 0.2f, 20});
    engine.GetEntity(ground).Color(0.3f, 0.3f, 0.3f).MakeStatic();

    // Some test objects
    int cube1 = engine.SpawnCube({-3, 3, 0});
    engine.GetEntity(cube1).Color(1.0f, 0.3f, 0.3f);

    int sphere1 = engine.SpawnSphere({0, 5, 0});
    engine.GetEntity(sphere1).Color(0.3f, 1.0f, 0.3f);

    int cube2 = engine.SpawnCube({3, 4, 0});
    engine.GetEntity(cube2).Color(0.3f, 0.3f, 1.0f);

    // Set camera position for good view
    engine.SetCameraPosition({10, 8, 10});
    engine.SetCameraTarget({0, 2, 0});

    // Update callback for editor
    engine.SetUpdateCallback([&](Engine& engine, float deltaTime) {
        // Render all panels
        panelManager.RenderAllPanels(*ui);
    });

    BS_INFO(LogCategory::EDITOR, "Editor ready - F1 to toggle panels, ESC to exit");
    BS_INFO(LogCategory::CORE, "Main loop starting...");

    int result = engine.Run();
    
    BS_INFO(LogCategory::CORE, "Editor session completed, shutting down...");
    Logger::Instance().Shutdown();
    
    return result;
}
