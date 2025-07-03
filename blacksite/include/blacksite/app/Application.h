#pragma once
#include "blacksite/core/Engine.h"

namespace Blacksite {

class Application {
public:
    Application(const std::string& name, int width = 1920, int height = 1080);
    virtual ~Application() = default;

    // Application lifecycle
    bool Initialize();
    int Run();
    void Shutdown();

    // Game-specific setup (override these)
    virtual void OnInitialize() {}
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnRender() {}
    virtual void OnShutdown() {}

protected:
    Engine& GetEngine() { return m_engine; }

private:
    std::string m_name;
    int m_width, m_height;
    Engine m_engine;

    void HandleInput();
};

}  // namespace Blacksite
