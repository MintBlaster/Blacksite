#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>
#include "blacksite/Blacksite.h"

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    Blacksite::Engine engine;

    if (!engine.Initialize(1920, 1080, "Blacksite: Physics Stress Test - How Much Can You Handle?")) {
        std::cerr << "Failed to initialize Blacksite Engine!" << std::endl;
        return -1;
    }

    std::cout << "=== BLACKSITE PHYSICS STRESS TEST INITIATED ===" << std::endl;
    std::cout << "User: MintBlaster | " << "2025-06-30 12:24:32 UTC" << std::endl;

    // === PERFORMANCE TRACKING ===
    auto startTime = std::chrono::high_resolution_clock::now();
    int totalObjectsSpawned = 0;
    int activeObjects = 0;
    float avgFPS = 60.0f;
    std::vector<float> frameTimeHistory;

    // === MASSIVE ARENA ===
    int ground = engine.SpawnPlane({0, -15, 0}, {100, 3, 100});
    engine.GetEntity(ground).Color(0.2f, 0.2f, 0.2f).MakeStatic();

    // === OBJECT STORAGE ===
    std::vector<int> cubes;
    std::vector<int> spheres;
    std::vector<int> stressObjects;

    // === INITIAL OBJECT WAVE - SPAWN ABOVE CENTER ===
    std::cout << "🚀 PHASE 1: Initial deployment of 50 objects..." << std::endl;
    for (int i = 0; i < 50; i++) {
        // Spawn in a small area above center so they fall and pile up
        float x = (rand() % 20 - 10);  // Much smaller spread
        float z = (rand() % 20 - 10);  // Much smaller spread
        float y = 20 + i * 0.5f;       // Stack them vertically

        if (i % 2 == 0) {
            int cube = engine.SpawnCube({x, y, z});
            engine.GetEntity(cube).Color(0.5f + (rand() % 50) / 100.0f, 0.5f + (rand() % 50) / 100.0f,
                                         0.5f + (rand() % 50) / 100.0f);
            cubes.push_back(cube);
        } else {
            int sphere = engine.SpawnSphere({x, y, z});
            engine.GetEntity(sphere).Color(0.5f + (rand() % 50) / 100.0f, 0.5f + (rand() % 50) / 100.0f,
                                           0.5f + (rand() % 50) / 100.0f);
            spheres.push_back(sphere);
        }
        totalObjectsSpawned++;
    }

    // === STATIC CAMERA - NO ROTATION ===
    engine.SetCameraPosition({35.0f, 25.0f, 35.0f});
    engine.SetCameraTarget({0, 0, 0});

    // === STRESS TEST VARIABLES ===
    float gameTime = 0.0f;
    float spawnTimer = 0.0f;
    float forceTimer = 0.0f;
    float performanceTimer = 0.0f;
    float stressPhaseTimer = 0.0f;
    int stressLevel = 1;
    int maxStressLevel = 10;
    float spawnRate = 2.0f;  // Start slow, get faster

    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    engine.SetUpdateCallback([&](Blacksite::Engine& engine, float deltaTime) {
        gameTime += deltaTime;
        spawnTimer += deltaTime;
        forceTimer += deltaTime;
        performanceTimer += deltaTime;
        stressPhaseTimer += deltaTime;

        // === FRAME TIME TRACKING ===
        auto currentTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float>(currentTime - lastFrameTime).count();
        lastFrameTime = currentTime;

        frameTimeHistory.push_back(frameTime);
        if (frameTimeHistory.size() > 60) {
            frameTimeHistory.erase(frameTimeHistory.begin());
        }

        // Calculate average FPS
        float avgFrameTime = 0.0f;
        for (float ft : frameTimeHistory) {
            avgFrameTime += ft;
        }
        avgFrameTime /= frameTimeHistory.size();
        avgFPS = 1.0f / avgFrameTime;

        // === STRESS LEVEL PROGRESSION ===
        if (stressPhaseTimer > 15.0f && stressLevel < maxStressLevel) {
            stressLevel++;
            spawnRate = std::max(0.1f, spawnRate * 0.7f);  // Spawn faster each level
            stressPhaseTimer = 0.0f;

            std::cout << "💀 STRESS LEVEL " << stressLevel << " ACTIVATED!" << std::endl;
            std::cout << "   Spawn rate: " << spawnRate << "s | Objects: " << totalObjectsSpawned << std::endl;

            // Special effects for each stress level
            switch (stressLevel) {
                case 2:
                    std::cout << "   🔥 FORCE MULTIPLIER ENABLED!" << std::endl;
                    break;
                case 3:
                    std::cout << "   ⚡ RAPID SPAWN MODE!" << std::endl;
                    break;
                case 4:
                    std::cout << "   💥 EXPLOSION CHAMBER!" << std::endl;
                    break;
                case 5:
                    std::cout << "   🌪️ TORNADO MODE!" << std::endl;
                    break;
                case 6:
                    std::cout << "   🚀 ROCKET BARRAGE!" << std::endl;
                    break;
                case 7:
                    std::cout << "   ⚛️ NUCLEAR PHYSICS!" << std::endl;
                    break;
                case 8:
                    std::cout << "   🌌 GRAVITY CHAOS!" << std::endl;
                    break;
                case 9:
                    std::cout << "   💀 DEATH MODE!" << std::endl;
                    break;
                case 10:
                    std::cout << "   🔥💀 MAXIMUM CARNAGE! 💀🔥" << std::endl;
                    break;
            }
        }

        // === STRESS SPAWNING - ALWAYS ABOVE CENTER ===
        if (spawnTimer > spawnRate) {
            int spawnCount = stressLevel;  // Spawn more objects as stress increases

            for (int i = 0; i < spawnCount; i++) {
                // Spawn directly above the pile with small random offset
                float x = (rand() % 8 - 4);    // Small spread around center
                float z = (rand() % 8 - 4);    // Small spread around center
                float y = 30 + (rand() % 10);  // High above so they fall down

                if (rand() % 2 == 0) {
                    int cube = engine.SpawnCube({x, y, z});
                    float intensity = stressLevel / 10.0f;
                    engine.GetEntity(cube).Color(intensity, 0.2f, 0.2f);
                    cubes.push_back(cube);
                } else {
                    int sphere = engine.SpawnSphere({x, y, z});
                    float intensity = stressLevel / 10.0f;
                    engine.GetEntity(sphere).Color(0.2f, intensity, 0.2f);
                    spheres.push_back(sphere);
                }
                totalObjectsSpawned++;
            }
            spawnTimer = 0.0f;
        }

        // === STRESS FORCES - TARGET THE PILE ===
        if (forceTimer > 1.0f) {
            int forceCount = stressLevel * 2;  // More forces as stress increases
            float forceMultiplier = 1.0f + (stressLevel * 0.5f);

            // Apply forces to random objects in the pile
            for (int i = 0; i < forceCount && !cubes.empty(); i++) {
                int randomCube = cubes[rand() % cubes.size()];
                glm::vec3 force = {(rand() % 1000 - 500) * forceMultiplier, (rand() % 500 + 100) * forceMultiplier,
                                   (rand() % 1000 - 500) * forceMultiplier};
                engine.GetEntity(randomCube).Push(force);
            }

            for (int i = 0; i < forceCount && !spheres.empty(); i++) {
                int randomSphere = spheres[rand() % spheres.size()];
                glm::vec3 impulse = {(rand() % 40 - 20) * forceMultiplier, (rand() % 20 + 10) * forceMultiplier,
                                     (rand() % 40 - 20) * forceMultiplier};
                engine.GetEntity(randomSphere).Impulse(impulse);
            }
            forceTimer = 0.0f;
        }

        // === SPECIAL STRESS EVENTS - AERIAL BOMBARDMENT ===
        if (stressLevel >= 4 && static_cast<int>(gameTime) % 5 == 0) {
            static int lastSpecialEvent = -1;
            if (lastSpecialEvent != static_cast<int>(gameTime)) {
                std::cout << "💥 AERIAL BOMBARDMENT: Objects raining down!" << std::endl;

                // Spawn a burst of objects high above center
                for (int i = 0; i < stressLevel * 3; i++) {
                    float x = (rand() % 12 - 6);   // Tight formation
                    float z = (rand() % 12 - 6);   // Tight formation
                    float y = 40 + (rand() % 15);  // Very high

                    int obj = engine.SpawnSphere({x, y, z});
                    engine.GetEntity(obj).Color(1.0f, 1.0f, 0.0f).SetVelocity({0, -20, 0});  // Fast downward velocity
                    spheres.push_back(obj);
                    totalObjectsSpawned++;
                }
                lastSpecialEvent = static_cast<int>(gameTime);
            }
        }

        // === PERFORMANCE MONITORING ===
        if (performanceTimer > 2.0f) {
            activeObjects = cubes.size() + spheres.size();

            std::cout << "📊 STRESS TEST STATUS:" << std::endl;
            std::cout << "   Level: " << stressLevel << "/" << maxStressLevel << std::endl;
            std::cout << "   FPS: " << static_cast<int>(avgFPS) << std::endl;
            std::cout << "   Active Objects: " << activeObjects << std::endl;
            std::cout << "   Total Spawned: " << totalObjectsSpawned << std::endl;
            std::cout << "   Runtime: " << static_cast<int>(gameTime) << "s" << std::endl;

            if (avgFPS < 20.0f) {
                std::cout << "⚠️  WARNING: Low FPS detected! Physics engine under stress!" << std::endl;
            } else if (avgFPS < 30.0f) {
                std::cout << "⚡ High load detected - physics working hard!" << std::endl;
            } else {
                std::cout << "✅ Performance good - ready for more stress!" << std::endl;
            }
            std::cout << "----------------------------------------" << std::endl;

            performanceTimer = 0.0f;
        }

        // === ULTIMATE STRESS TEST - CONTINUOUS RAIN ===
        if (stressLevel >= maxStressLevel) {
            static bool ultimateTriggered = false;
            if (!ultimateTriggered) {
                std::cout << "🔥💀🔥 ULTIMATE STRESS TEST: ENDLESS OBJECT RAIN! 🔥💀🔥" << std::endl;
                std::cout << "PREPARE FOR MAXIMUM PHYSICS PILE-UP!" << std::endl;
                ultimateTriggered = true;
            }

            // Continuous raining objects at max level
            if (static_cast<int>(gameTime * 10) % 3 == 0) {
                for (int i = 0; i < 8; i++) {
                    float x = (rand() % 6 - 3);    // Very tight center spawn
                    float z = (rand() % 6 - 3);    // Very tight center spawn
                    float y = 35 + (rand() % 10);  // High drop

                    int obj = engine.SpawnCube({x, y, z});
                    engine.GetEntity(obj).Color(1.0f, 0.0f, 0.0f);
                    cubes.push_back(obj);
                    totalObjectsSpawned++;
                }
            }
        }
    });

    std::cout << "\n🔥💀 BLACKSITE PHYSICS STRESS TEST 💀🔥\n";
    std::cout << "MintBlaster's Ultimate Object Pile-Up Challenge\n";
    std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n";
    std::cout << "📈 Progressive stress levels (1-10)\n";
    std::cout << "📦 Objects spawn above center and fall into pile\n";
    std::cout << "🎯 Static camera for clear pile observation\n";
    std::cout << "⚡ Increasing spawn rates and forces\n";
    std::cout << "📊 Real-time performance monitoring\n";
    std::cout << "💥 Aerial bombardment events\n";
    std::cout << "🏔️ Watch the physics pile grow!\n";
    std::cout << "\nPress ESC when your pile reaches critical mass!\n\n";

    return engine.Run();
}