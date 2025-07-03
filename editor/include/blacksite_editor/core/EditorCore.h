#pragma once
#include <blacksite/core/Engine.h>
#include <string>

namespace BlacksiteEditor {

class EditorCore {
  public:
    EditorCore(Blacksite::Engine* engine);
    ~EditorCore() = default;

    void Initialize();
    void Update(float deltaTime);
    void Shutdown();

    // Engine access
    Blacksite::Engine* GetEngine() { return m_engine; }
    Blacksite::Scene* GetActiveScene();

    // Selection management
    void SetSelectedEntity(int entityId) { m_selectedEntityId = entityId; }
    int GetSelectedEntity() const { return m_selectedEntityId; }
    bool HasSelectedEntity() const { return m_selectedEntityId >= 0; }

    // Scene management
    void NewScene(const std::string& name);
    void SaveScene(const std::string& filename);
    void LoadScene(const std::string& filename);

    // Entity management
    int CreateEntity(const std::string& type);
    void DeleteEntity(int entityId);
    int DuplicateEntity(int entityId);
    void ClearScene();
    void SelectEntity(int entityId);

    // Editor settings
    bool IsGridVisible() const { return m_showGrid; }
    void SetGridVisible(bool visible) { m_showGrid = visible; }

    bool IsGizmosVisible() const { return m_showGizmos; }
    void SetGizmosVisible(bool visible) { m_showGizmos = visible; }

  private:
    Blacksite::Engine* m_engine;
    int m_selectedEntityId = -1;

    // Editor settings
    bool m_showGrid = true;
    bool m_showGizmos = true;

    // Performance tracking
    float m_frameTime = 0.0f;
    int m_frameCount = 0;
    float m_fps = 0.0f;

    int m_selectedEntity = -1;

    void UpdatePerformanceStats(float deltaTime);
};

}  // namespace BlacksiteEditor
