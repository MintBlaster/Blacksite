#include <iostream>
#include "blacksite/blacksite.h"

int main()
{
    Blacksite::Engine engine;

    // Boot up the engine
    if (!engine.Initialize(1280, 720, "Blacksite Renderer Test"))
    {
        std::cerr << "Failed to initialize engine" << std::endl;
        return -1;
    }

    // Drop a fat red cube in the center
    auto cube = engine.GetEntity(engine.SpawnCube({0, 0, 0})).Color(1.0f, 0.2f, 0.2f).Scale(1.5f);

    // Squashed sphere on the left
    auto sphere = engine.GetEntity(engine.SpawnSphere({-3, 0, 0})).Color(0.2f, 1.0f, 0.2f).Scale({2, 1, 2});

    // Ground plane — does absolutely nothing
    auto plane = engine.GetEntity(engine.SpawnPlane({0, -2, 0})).Color(0.8f, 0.8f, 0.8f).Scale({10, 1, 10});

    // Rotated cube just to prove transforms work
    engine.GetEntity(engine.SpawnCube({3, 1, 0})).Color(0.2f, 0.2f, 1.0f).Rotate({45, 0, 45}).Scale(0.8f);

    // Main loop
    return engine.Run();
}
