#include <cstdlib>
#include <ctime>
#include <vector>
#include "blacksite/Blacksite.h"

using namespace Blacksite;

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Initialize logger first
    Logger::Instance().Initialize();
    BS_INFO(LogCategory::CORE, "Blacksite Basic Physics Demo");
    BS_INFO(LogCategory::CORE, "Demo initialization starting...");

    Engine engine;

    if (!engine.Initialize(1920, 1080, "Blacksite: Basic Physics Demo - MintBlaster")) {
        BS_ERROR(LogCategory::CORE, "Engine initialization failed");
        return -1;
    }

    BS_INFO(LogCategory::GAME, "Scene setup starting...");

    // === SIMPLE SCENE SETUP ===

    // Ground plane
    int ground = engine.SpawnPlane({0, -2, 0}, {30, 1, 30});
    engine.GetEntity(ground).Color(0.3f, 0.3f, 0.3f).MakeStatic();

    // A few static walls for bouncing
    int wallLeft = engine.SpawnCube({-15, 5, 0});
    engine.GetEntity(wallLeft).Scale(1, 10, 30).Color(0.4f, 0.2f, 0.2f).MakeStatic();

    int wallRight = engine.SpawnCube({15, 5, 0});
    engine.GetEntity(wallRight).Scale(1, 10, 30).Color(0.2f, 0.4f, 0.2f).MakeStatic();

    int wallBack = engine.SpawnCube({0, 5, -15});
    engine.GetEntity(wallBack).Scale(30, 10, 1).Color(0.2f, 0.2f, 0.4f).MakeStatic();

    // Some starting objects
    std::vector<int> movingCubes;
    std::vector<int> movingSpheres;

    // Stack of cubes
    for (int i = 0; i < 5; ++i) {
        int cube = engine.SpawnCube({-5, 2 + i * 2.2f, 0});
        engine.GetEntity(cube).Color(0.8f, 0.4f + i * 0.1f, 0.2f);
        movingCubes.push_back(cube);
    }

    // Some bouncing spheres
    for (int i = 0; i < 3; ++i) {
        int sphere = engine.SpawnSphere({5 + i * 3, 8, -5 + i * 2});
        engine.GetEntity(sphere).Color(0.2f, 0.7f, 0.8f + i * 0.1f);
        movingSpheres.push_back(sphere);
    }

    // Set camera for good view
    engine.SetCameraPosition({25, 15, 25});
    engine.SetCameraTarget({0, 5, 0});

    // === SIMPLE ANIMATION VARIABLES ===
    float gameTime = 0.0f;
    float pushTimer = 0.0f;
    float spawnTimer = 0.0f;
    int totalSpawned = movingCubes.size() + movingSpheres.size();

    engine.SetUpdateCallback([&](Blacksite::Engine& engine, float deltaTime) {
        gameTime += deltaTime;
        pushTimer += deltaTime;
        spawnTimer += deltaTime;

        // Give objects a random push every few seconds
        if (pushTimer > 4.0f) {
            if (!movingCubes.empty()) {
                int randomCube = movingCubes[rand() % movingCubes.size()];
                glm::vec3 randomForce = {(rand() % 1000 - 500), rand() % 500 + 200, (rand() % 1000 - 500)};
                engine.GetEntity(randomCube).Push(randomForce);
            }

            if (!movingSpheres.empty()) {
                int randomSphere = movingSpheres[rand() % movingSpheres.size()];
                glm::vec3 randomImpulse = {(rand() % 20 - 10), rand() % 15 + 5, (rand() % 20 - 10)};
                engine.GetEntity(randomSphere).Impulse(randomImpulse);
            }

            pushTimer = 0.0f;
            BS_INFO_F(LogCategory::PHYSICS, "Random forces applied (runtime: %ds)", (int)gameTime);
        }

        // Occasionally spawn a new object
        if (spawnTimer > 6.0f && totalSpawned < 20) {
            if (rand() % 2 == 0) {
                // Spawn cube above the scene
                int newCube = engine.SpawnCube({(rand() % 20 - 10), 15, (rand() % 20 - 10)});
                engine.GetEntity(newCube).Color(0.3f + (rand() % 50) / 100.0f, 0.3f + (rand() % 50) / 100.0f,
                                                0.3f + (rand() % 50) / 100.0f);
                movingCubes.push_back(newCube);
            } else {
                // Spawn sphere above the scene
                int newSphere = engine.SpawnSphere({(rand() % 20 - 10), 15, (rand() % 20 - 10)});
                engine.GetEntity(newSphere).Color(0.4f + (rand() % 40) / 100.0f, 0.6f + (rand() % 30) / 100.0f,
                                                  0.5f + (rand() % 40) / 100.0f);
                movingSpheres.push_back(newSphere);
            }

            totalSpawned++;
            spawnTimer = 0.0f;
            BS_INFO_F(LogCategory::GAME, "Entity spawned (total: %d)", totalSpawned);
        }

        // Simple status update
        if ((int)gameTime % 10 == 0 && (int)(gameTime * 10) % 10 == 0) {
            static int lastUpdate = -1;
            if (lastUpdate != (int)gameTime) {
                BS_INFO_F(LogCategory::GAME, "Scene tick (runtime: %ds, entities: %d)", (int)gameTime, totalSpawned);
                lastUpdate = (int)gameTime;
            }
        }
    });

    BS_INFO(LogCategory::CORE, "Controls: ESC to exit");
    BS_INFO(LogCategory::CORE, "Main loop starting...");

    int result = engine.Run();
    
    BS_INFO(LogCategory::CORE, "Demo completed, shutting down...");
    Logger::Instance().Shutdown();
    
    return result;
}
