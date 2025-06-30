#include <cmath>
#include <cstdlib>
#include <iostream>
#include "blacksite/Blacksite.h"

int main() {
    std::cout << "Blacksite Engine - Spinning Cube with Orbiting Sphere" << std::endl;

    Blacksite::Engine engine;

    if (!engine.Initialize(1280, 720, "Blacksite Engine - Dynamic Objects")) {
        std::cerr << "Engine initialization failed!" << std::endl;
        return EXIT_FAILURE;
    }

    // Set up camera
    engine.SetCameraPosition({0.0f, 3.0f, 8.0f});
    engine.SetCameraTarget({0.0f, 0.0f, 0.0f});

    // Spawn objects
    int cubeId = engine.SpawnCube({0.0f, 0.0f, 0.0f});
    int sphereId = engine.SpawnSphere({3.0f, 0.0f, 0.0f});
    int groundId = engine.SpawnPlane({0.0f, -2.0f, 0.0f});

    // Set initial colors and scales
    engine.GetEntity(cubeId).Color(1.0f, 0.2f, 0.2f).Scale(1.5f);
    engine.GetEntity(sphereId).Color(0.2f, 0.8f, 1.0f).Scale(0.8f);
    engine.GetEntity(groundId).Color(0.3f, 0.6f, 0.3f).Scale(10.0f, 1.0f, 10.0f);

    // Game state
    float gameTime = 0.0f;
    float cubeRotationSpeed = 45.0f;  // degrees per second
    float sphereOrbitSpeed = 60.0f;   // degrees per second
    float orbitRadius = 3.0f;

    // Set up the update callback - THIS MAKES THINGS MOVE!
    engine.SetUpdateCallback([&](Blacksite::Engine& engine, float deltaTime) {
        gameTime += deltaTime;

        // Spin the cube around Y axis
        float cubeRotation = gameTime * cubeRotationSpeed;
        engine.GetEntity(cubeId).Rotate({0.0f, cubeRotation, 0.0f});

        // Make the sphere orbit around the cube
        float orbitAngle = gameTime * sphereOrbitSpeed * (3.14159f / 180.0f);  // Convert to radians
        float sphereX = cos(orbitAngle) * orbitRadius;
        float sphereZ = sin(orbitAngle) * orbitRadius;
        float sphereY = sin(gameTime * 2.0f) * 0.5f;  // Add some vertical bobbing

        engine.GetEntity(sphereId).At({sphereX, sphereY, sphereZ});

        // Make the sphere change color over time
        float r = (sin(gameTime * 2.0f) + 1.0f) * 0.5f;
        float g = (cos(gameTime * 1.5f) + 1.0f) * 0.5f;
        float b = (sin(gameTime * 3.0f) + 1.0f) * 0.5f;
        engine.GetEntity(sphereId).Color(r, g, b);

        // Make the cube pulse in size
        float scaleMultiplier = 1.5f + sin(gameTime * 4.0f) * 0.3f;
        engine.GetEntity(cubeId).Scale(scaleMultiplier);

        // Rotate the camera around the scene for a dynamic view
        float cameraAngle = gameTime * 10.0f * (3.14159f / 180.0f);  // Slow camera rotation
        float cameraX = cos(cameraAngle) * 8.0f;
        float cameraZ = sin(cameraAngle) * 8.0f;
        engine.SetCameraPosition({cameraX, 3.0f, cameraZ});
        engine.SetCameraTarget({0.0f, 0.0f, 0.0f});
    });
    std::cout << "Press ESC to exit." << std::endl;

    return engine.Run();
}