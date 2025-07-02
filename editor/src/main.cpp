#include "Editor.h"
#include "blacksite/core/Engine.h"
#include <iostream>
#include <chrono>
#include <random>
#include <cmath>

class SolarSystemScene : public Blacksite::Scene {
public:
    SolarSystemScene(const std::string& name) : Scene(name) {}

    void OnEnter() override {
        BS_INFO(Blacksite::LogCategory::EDITOR, "🌟 Entering Solar System Scene...");
        SetupSolarSystem();
        m_timeScale = 1.0f;
        m_systemTime = 0.0f;
        m_cinematicMode = false;
        m_cameraAngle = 0.0f;
    }

    void OnExit() override {
        BS_INFO(Blacksite::LogCategory::EDITOR, "🚀 Exiting Solar System Scene...");
    }

    void Update(float deltaTime) override {
        Scene::Update(deltaTime);

        m_systemTime += deltaTime * m_timeScale;

        UpdatePlanetaryMotion(deltaTime);
        UpdateAsteroidBelt(deltaTime);
        UpdateComets(deltaTime);

        if (m_cinematicMode) {
            UpdateCinematicCamera(deltaTime);
        }

        // Pulse the sun
        UpdateSunPulse(deltaTime);
    }

    // Public control methods
    void SpeedUpTime() {
        m_timeScale = std::min(m_timeScale * 2.0f, 16.0f);
        BS_INFO_F(Blacksite::LogCategory::EDITOR, "Time scale: %.1fx", m_timeScale);
    }

    void SlowDownTime() {
        m_timeScale = std::max(m_timeScale * 0.5f, 0.25f);
        BS_INFO_F(Blacksite::LogCategory::EDITOR, "Time scale: %.1fx", m_timeScale);
    }

    void ResetTime() {
        m_timeScale = 1.0f;
        BS_INFO(Blacksite::LogCategory::EDITOR, "Time scale reset to 1.0x");
    }

    void ToggleCinematicMode() {
        m_cinematicMode = !m_cinematicMode;
        BS_INFO_F(Blacksite::LogCategory::EDITOR, "Cinematic mode: %s", m_cinematicMode ? "ON" : "OFF");
    }

    void CreateMeteorShower() {
        BS_INFO(Blacksite::LogCategory::EDITOR, "☄️ Meteor Shower!");

        for (int i = 0; i < 15; i++) {
            glm::vec3 startPos(
                RandomFloat(-80.0f, 80.0f),
                RandomFloat(40.0f, 60.0f),
                RandomFloat(-80.0f, 80.0f)
            );

            glm::vec3 meteorColor(1.0f, 0.7f, 0.3f);
            int meteor = SpawnSphere(startPos, "unlit", meteorColor);

            glm::vec3 velocity(
                RandomFloat(-15.0f, 15.0f),
                RandomFloat(-25.0f, -15.0f),
                RandomFloat(-15.0f, 15.0f)
            );

            GetEntity(meteor).Scale(RandomFloat(0.3f, 0.8f))
                            .MakeDynamic()
                            .SetVelocity(velocity)
                            .SetAngularVelocity(glm::vec3(RandomFloat(-10.f, 10.f),
                                                        RandomFloat(-10.f, 10.f),
                                                        RandomFloat(-10.f, 10.f)));

            // Create trail particles
            CreateMeteorTrail(startPos, velocity, i * 0.1f);
        }
    }

    void CreateSpaceDebris() {
        BS_INFO(Blacksite::LogCategory::EDITOR, "🛰️ Space Debris Field!");

        for (int i = 0; i < 25; i++) {
            float radius = RandomFloat(25.0f, 45.0f);
            float angle = RandomFloat(0, 2.0f * M_PI);
            float height = RandomFloat(-10.0f, 10.0f);

            glm::vec3 pos(
                cos(angle) * radius,
                height,
                sin(angle) * radius
            );

            glm::vec3 debrisColor(0.6f, 0.6f, 0.7f);
            int debris;

            if (i % 3 == 0) {
                debris = SpawnCube(pos, "basic", debrisColor);
                GetEntity(debris).Scale(RandomFloat(0.2f, 1.0f),
                                      RandomFloat(0.2f, 0.5f),
                                      RandomFloat(0.2f, 1.0f));
            } else {
                debris = SpawnSphere(pos, "unlit", debrisColor);
                GetEntity(debris).Scale(RandomFloat(0.3f, 0.8f));
            }

            // Orbital motion around center
            glm::vec3 orbitalVel(-sin(angle) * 3.0f, 0, cos(angle) * 3.0f);
            GetEntity(debris).MakeDynamic()
                            .SetVelocity(orbitalVel)
                            .SetAngularVelocity(glm::vec3(RandomFloat(-5.f, 5.f),
                                                        RandomFloat(-5.f, 5.f),
                                                        RandomFloat(-5.f, 5.f)));
        }
    }

    void TriggerSolarFlare() {
        BS_INFO(Blacksite::LogCategory::EDITOR, "☀️ Solar Flare!");

        // Create flare particles radiating from the sun
        glm::vec3 sunPos(0, 0, 0);

        for (int i = 0; i < 30; i++) {
            float angle1 = RandomFloat(0, 2.0f * M_PI);
            float angle2 = RandomFloat(-M_PI/3, M_PI/3);

            glm::vec3 direction(
                cos(angle1) * cos(angle2),
                sin(angle2),
                sin(angle1) * cos(angle2)
            );

            glm::vec3 startPos = sunPos + direction * 2.5f;
            glm::vec3 flareColor(1.0f, 0.8f, 0.2f);

            int flare = SpawnSphere(startPos, "unlit", flareColor);

            glm::vec3 velocity = direction * RandomFloat(8.0f, 15.0f);
            GetEntity(flare).Scale(RandomFloat(0.2f, 0.6f))
                           .MakeDynamic()
                           .SetVelocity(velocity);
        }
    }

    void ClearDynamicObjects() {
            BS_INFO(Blacksite::LogCategory::EDITOR, "🧹 Clearing dynamic objects...");
            auto& entities = GetEntitySystem()->GetEntities();
            for (size_t i = 0; i < entities.size(); ++i) {
                auto& e = entities[i];
                if (e.active && e.hasPhysics && e.isDynamic) {
                    // Don't clear planets and moons (they have specific IDs we track)
                    bool isPlanetaryBody = false;
                    for (const auto& planet : m_planets) {
                        if (static_cast<int>(i) == planet.entityId ||
                            std::find(planet.moonIds.begin(), planet.moonIds.end(), static_cast<int>(i)) != planet.moonIds.end()) {
                            isPlanetaryBody = true;
                            break;
                        }
                    }
                    if (!isPlanetaryBody) {
                        e.active = false;
                    }
                }
            }
        }

private:
    struct Planet {
        int entityId;
        float orbitRadius;
        float orbitSpeed;
        float rotationSpeed;
        glm::vec3 color;
        std::vector<int> moonIds;
        std::vector<float> moonOrbitRadius;
        std::vector<float> moonOrbitSpeed;
    };

    std::vector<Planet> m_planets;
    std::vector<int> m_asteroids;
    std::vector<int> m_comets;
    std::vector<glm::vec3> m_cometOrbits;

    float m_timeScale;
    float m_systemTime;
    bool m_cinematicMode;
    float m_cameraAngle;
    int m_sunId;

    std::mt19937 rng{std::random_device{}()};

    float RandomFloat(float min, float max) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(rng);
    }

    void SetupSolarSystem() {
        BS_INFO(Blacksite::LogCategory::EDITOR, "🌌 Creating Solar System...");

        // Create the Sun at center
        glm::vec3 sunColor(1.0f, 0.9f, 0.3f);
        m_sunId = SpawnSphere(glm::vec3(0, 0, 0), "unlit", sunColor);
        GetEntity(m_sunId).Scale(2.5f).MakeStatic();

        // Create planets with realistic-ish proportions and colors
        CreatePlanet("Mercury", 8.0f, 2.5f, 4.0f, glm::vec3(0.7f, 0.6f, 0.5f), 0.4f);
        CreatePlanet("Venus", 12.0f, 1.8f, 3.0f, glm::vec3(1.0f, 0.8f, 0.3f), 0.6f);
        CreatePlanet("Earth", 16.0f, 1.2f, 2.0f, glm::vec3(0.2f, 0.5f, 1.0f), 0.8f);
        CreatePlanet("Mars", 22.0f, 0.8f, 1.5f, glm::vec3(0.8f, 0.4f, 0.2f), 0.6f);
        CreatePlanet("Jupiter", 35.0f, 0.4f, 0.8f, glm::vec3(0.9f, 0.7f, 0.5f), 1.8f);
        CreatePlanet("Saturn", 45.0f, 0.3f, 0.6f, glm::vec3(0.8f, 0.8f, 0.6f), 1.5f);
        CreatePlanet("Uranus", 55.0f, 0.2f, 0.4f, glm::vec3(0.3f, 0.7f, 0.8f), 1.0f);
        CreatePlanet("Neptune", 65.0f, 0.15f, 0.3f, glm::vec3(0.2f, 0.4f, 0.9f), 1.0f);

        // Add moons to Earth
        AddMoonToPlanet(2, 2.5f, 3.0f, glm::vec3(0.8f, 0.8f, 0.8f), 0.2f); // Earth's moon

        // Add moons to Jupiter
        AddMoonToPlanet(4, 3.0f, 4.0f, glm::vec3(0.9f, 0.8f, 0.6f), 0.15f);
        AddMoonToPlanet(4, 4.0f, 2.5f, glm::vec3(0.8f, 0.7f, 0.5f), 0.12f);
        AddMoonToPlanet(4, 5.5f, 1.8f, glm::vec3(0.7f, 0.6f, 0.4f), 0.1f);

        // Add Saturn's rings (simplified as small particles)
        CreateSaturnRings();

        // Create asteroid belt between Mars and Jupiter
        CreateAsteroidBelt();

        // Create some comets
        CreateComets();

        // Set initial camera position
        SetCameraPosition(glm::vec3(30.f, 25.f, 30.f));
        SetCameraTarget(glm::vec3(0, 0, 0));

        BS_INFO(Blacksite::LogCategory::EDITOR, "✨ Solar System Complete! 8 planets, moons, asteroids, and comets ready");
    }

    void CreatePlanet(const std::string& name, float orbitRadius, float orbitSpeed,
                     float rotSpeed, const glm::vec3& color, float scale) {
        Planet planet;
        planet.orbitRadius = orbitRadius;
        planet.orbitSpeed = orbitSpeed;
        planet.rotationSpeed = rotSpeed;
        planet.color = color;

        // Start at a random position in orbit
        float initialAngle = RandomFloat(0, 2.0f * M_PI);
        glm::vec3 pos(cos(initialAngle) * orbitRadius, 0, sin(initialAngle) * orbitRadius);

        planet.entityId = SpawnSphere(pos, "basic", color);
        GetEntity(planet.entityId).Scale(scale).MakeDynamic();

        m_planets.push_back(planet);

        BS_INFO_F(Blacksite::LogCategory::EDITOR, "🪐 Created %s at orbit %.1f", name.c_str(), orbitRadius);
    }

    void AddMoonToPlanet(int planetIndex, float moonOrbitRadius, float moonOrbitSpeed,
                        const glm::vec3& color, float scale) {
        if (planetIndex >= m_planets.size()) return;

        Planet& planet = m_planets[planetIndex];

        // Create moon at random position around planet
        float angle = RandomFloat(0, 2.0f * M_PI);
        glm::vec3 planetPos = GetEntity(planet.entityId).GetPosition();
        glm::vec3 moonPos = planetPos + glm::vec3(cos(angle) * moonOrbitRadius, 0, sin(angle) * moonOrbitRadius);

        int moonId = SpawnSphere(moonPos, "basic", color);
        GetEntity(moonId).Scale(scale).MakeDynamic();

        planet.moonIds.push_back(moonId);
        planet.moonOrbitRadius.push_back(moonOrbitRadius);
        planet.moonOrbitSpeed.push_back(moonOrbitSpeed);
    }

    void CreateSaturnRings() {
        // Create ring particles around Saturn (planet index 5)
        if (m_planets.size() <= 5) return;

        glm::vec3 saturnPos = GetEntity(m_planets[5].entityId).GetPosition();

        for (int i = 0; i < 50; i++) {
            float radius = RandomFloat(2.5f, 4.0f);
            float angle = RandomFloat(0, 2.0f * M_PI);

            glm::vec3 ringPos = saturnPos + glm::vec3(cos(angle) * radius,
                                                     RandomFloat(-0.1f, 0.1f),
                                                     sin(angle) * radius);

            glm::vec3 ringColor(0.9f, 0.8f, 0.7f);
            int particle = SpawnSphere(ringPos, "basic", ringColor);
            GetEntity(particle).Scale(0.05f).MakeDynamic();

            // Give orbital motion
            glm::vec3 velocity(-sin(angle) * 2.0f, 0, cos(angle) * 2.0f);
            GetEntity(particle).SetVelocity(velocity);
        }
    }

    void CreateAsteroidBelt() {
        for (int i = 0; i < 40; i++) {
            float radius = RandomFloat(28.0f, 32.0f); // Between Mars and Jupiter
            float angle = RandomFloat(0, 2.0f * M_PI);
            float height = RandomFloat(-2.0f, 2.0f);

            glm::vec3 pos(cos(angle) * radius, height, sin(angle) * radius);
            glm::vec3 asteroidColor(0.5f, 0.4f, 0.3f);

            int asteroid = SpawnCube(pos, "basic", asteroidColor);
            GetEntity(asteroid).Scale(RandomFloat(0.1f, 0.3f),
                                    RandomFloat(0.1f, 0.4f),
                                    RandomFloat(0.1f, 0.3f))
                              .MakeDynamic();

            // Orbital motion
            glm::vec3 velocity(-sin(angle) * 1.0f, 0, cos(angle) * 1.0f);
            GetEntity(asteroid).SetVelocity(velocity)
                              .SetAngularVelocity(glm::vec3(RandomFloat(-2, 2),
                                                          RandomFloat(-2, 2),
                                                          RandomFloat(-2, 2)));

            m_asteroids.push_back(asteroid);
        }
    }

    void CreateComets() {
        for (int i = 0; i < 3; i++) {
            // Highly elliptical orbits
            float maxRadius = RandomFloat(70.0f, 100.0f);
            float minRadius = RandomFloat(15.0f, 25.0f);
            float angle = RandomFloat(0, 2.0f * M_PI);

            glm::vec3 pos(cos(angle) * maxRadius,
                         RandomFloat(-5.0f, 5.0f),
                         sin(angle) * maxRadius);

            glm::vec3 cometColor(0.8f, 0.9f, 1.0f);
            int comet = SpawnSphere(pos, "unlit", cometColor);
            GetEntity(comet).Scale(0.4f).MakeDynamic();

            m_comets.push_back(comet);
            m_cometOrbits.push_back(glm::vec3(maxRadius, minRadius, angle));
        }
    }

    void UpdatePlanetaryMotion(float deltaTime) {
        for (size_t i = 0; i < m_planets.size(); i++) {
            Planet& planet = m_planets[i];

            // Update planet orbit
            float angle = m_systemTime * planet.orbitSpeed;
            glm::vec3 newPos(cos(angle) * planet.orbitRadius, 0, sin(angle) * planet.orbitRadius);
            GetEntity(planet.entityId).At(newPos);

            // Update planet rotation
            glm::vec3 rotationAxis(0, 1, 0);
            float rotationAngle = m_systemTime * planet.rotationSpeed;
            GetEntity(planet.entityId).Rotate(glm::vec3(0, rotationAngle, 0));

            // Update moons
            for (size_t j = 0; j < planet.moonIds.size(); j++) {
                float moonAngle = m_systemTime * planet.moonOrbitSpeed[j];
                glm::vec3 moonOffset(cos(moonAngle) * planet.moonOrbitRadius[j],
                                   0,
                                   sin(moonAngle) * planet.moonOrbitRadius[j]);
                glm::vec3 moonPos = newPos + moonOffset;
                GetEntity(planet.moonIds[j]).At(moonPos);
            }
        }
    }

    void UpdateAsteroidBelt(float deltaTime) {
        // Asteroids are handled by physics, just make sure they stay in rough orbit
        for (int asteroidId : m_asteroids) {
            if (!GetEntity(asteroidId).IsActive()) continue;

            glm::vec3 pos = GetEntity(asteroidId).GetPosition();
            float dist = glm::length(pos);

            // If asteroid drifts too far, give it a nudge back toward belt
            if (dist > 35.0f || dist < 25.0f) {
                glm::vec3 direction = glm::normalize(pos);
                glm::vec3 targetPos = direction * 30.0f; // Target belt radius
                glm::vec3 correctionForce = (targetPos - pos) * 0.5f;
                GetEntity(asteroidId).SetVelocity(GetEntity(asteroidId).GetVelocity() + correctionForce * deltaTime);
            }
        }
    }

    void UpdateComets(float deltaTime) {
        for (size_t i = 0; i < m_comets.size(); i++) {
            if (i >= m_cometOrbits.size()) continue;

            glm::vec3& orbit = m_cometOrbits[i];
            float maxRadius = orbit.x;
            float minRadius = orbit.y;
            float baseAngle = orbit.z;

            // Elliptical orbit calculation
            float t = m_systemTime * 0.1f + baseAngle;
            float a = (maxRadius + minRadius) * 0.5f; // Semi-major axis
            float e = (maxRadius - minRadius) / (maxRadius + minRadius); // Eccentricity

            float radius = a * (1 - e * e) / (1 + e * cos(t));
            glm::vec3 pos(cos(t) * radius, sin(t * 0.3f) * 3.0f, sin(t) * radius);

            GetEntity(m_comets[i]).At(pos);

            // Create tail effect when close to sun
            if (radius < 30.0f) {
                CreateCometTail(pos, i);
            }
        }
    }

    void CreateCometTail(const glm::vec3& cometPos, int cometIndex) {
        // Simple tail effect - spawn fading particles behind comet
        glm::vec3 tailColor(0.6f, 0.8f, 1.0f);
        glm::vec3 tailPos = cometPos + glm::vec3(RandomFloat(-1, 1), RandomFloat(-1, 1), RandomFloat(-1, 1));

        int tail = SpawnSphere(tailPos, "transparent", tailColor);
        GetEntity(tail).Scale(0.1f).MakeDynamic().SetVelocity(glm::vec3(RandomFloat(-2, 2), RandomFloat(-2, 2), RandomFloat(-2, 2)));
    }

    void CreateMeteorTrail(const glm::vec3& startPos, const glm::vec3& velocity, float delay) {
        // Create a trail of small particles behind the meteor
        for (int i = 0; i < 5; i++) {
            glm::vec3 trailPos = startPos - velocity * (i * 0.2f);
            glm::vec3 trailColor(1.0f, 0.6f, 0.2f);

            int trail = SpawnSphere(trailPos, "transparent", trailColor);
            GetEntity(trail).Scale(0.1f).MakeDynamic().SetVelocity(velocity * 0.8f);
        }
    }

    void UpdateSunPulse(float deltaTime) {
        // Make the sun pulse with energy
        float pulse = 1.0f + sin(m_systemTime * 2.0f) * 0.1f;
        GetEntity(m_sunId).Scale(2.5f * pulse);
    }

    void UpdateCinematicCamera(float deltaTime) {
        m_cameraAngle += deltaTime * 0.3f;

        // Orbit around the solar system with varying height and distance
        float radius = 40.0f + sin(m_cameraAngle * 0.5f) * 20.0f;
        float height = 15.0f + sin(m_cameraAngle * 0.7f) * 10.0f;

        glm::vec3 cameraPos(
            cos(m_cameraAngle) * radius,
            height,
            sin(m_cameraAngle) * radius
        );

        // Sometimes focus on specific planets
        glm::vec3 target(0, 0, 0);
        if (m_planets.size() > 0) {
            int focusPlanet = int(m_cameraAngle * 0.1f) % m_planets.size();
            if (sin(m_cameraAngle * 0.2f) > 0.5f) {
                target = GetEntity(m_planets[focusPlanet].entityId).GetPosition();
            }
        }

        SetCameraPosition(cameraPos);
        SetCameraTarget(target);
    }
};

int main() {
    Blacksite::Logger::Instance().Initialize();

    std::cout << "🌌 Starting Blacksite Solar System Simulation\n";
    BS_INFO(Blacksite::LogCategory::EDITOR, "🚀 Blacksite Solar System - Interactive Planetary Motion");

    Blacksite::Engine engine;
    if (!engine.Initialize(1920, 1080, "Blacksite Solar System Explorer")) return -1;

    // Configure post-processing for space aesthetics
    auto* renderer = engine.GetRenderer();
    if (renderer && renderer->GetPostProcessManager()) {
        auto& settings = renderer->GetPostProcessManager()->GetSettings();
        settings.enableToneMapping = true;
        settings.enableBloom = true;
        settings.enableFXAA = true;
        settings.exposure = 1.0f;
        settings.gamma = 2.2f;
        settings.bloomThreshold = 0.6f;
        settings.bloomStrength = 1.2f;

        BS_INFO(Blacksite::LogCategory::EDITOR, "Space rendering configured: Enhanced bloom for stellar effects");
    }

    GLFWwindow* window = engine.GetGLFWWindow();
    if (!window) return -1;

    auto solarScene = engine.CreateScene<SolarSystemScene>("SolarSystem");
    if (!solarScene || !engine.SwitchToScene("SolarSystem")) return -1;

    BlacksiteEditor::Editor editor;
    if (!editor.Initialize(&engine, window)) return -1;

    std::cout << "🎛️ Solar System Controls:\n";
    std::cout << "   [F1] Toggle Editor | [ESC] Exit | [C] Cinematic Camera\n";
    std::cout << "   [+] Speed Up Time | [-] Slow Down Time | [R] Reset Time\n";
    std::cout << "   [1] Meteor Shower | [2] Space Debris | [3] Solar Flare\n";
    std::cout << "   [SPACE] Clear Dynamic Objects | [F6] Toggle Post-Processing\n";
    std::cout << "🪐 Watch the planets orbit! Time controls affect orbital speed.\n";

    auto lastTime = std::chrono::high_resolution_clock::now();
    while (!glfwWindowShouldClose(window)) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);

        // Editor toggle
        static bool f1Pressed = false;
        bool f1Current = glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS;
        if (f1Current && !f1Pressed) editor.ToggleEnabled();
        f1Pressed = f1Current;

        // Post-processing toggle
        static bool f6Pressed = false;
        bool f6Current = glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS;
        if (f6Current && !f6Pressed) {
            auto* renderer = engine.GetRenderer();
            renderer->EnablePostProcessing(!renderer->IsPostProcessingEnabled());
            BS_INFO_F(Blacksite::LogCategory::EDITOR, "Post-processing: %s",
                      renderer->IsPostProcessingEnabled() ? "ENABLED" : "DISABLED");
        }
        f6Pressed = f6Current;

        // Time controls
        static bool plusPressed = false, minusPressed = false, rPressed = false;
        bool plusCurrent = glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS;
        bool minusCurrent = glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS;
        bool rCurrent = glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS;

        if (plusCurrent && !plusPressed) solarScene->SpeedUpTime(); plusPressed = plusCurrent;
        if (minusCurrent && !minusPressed) solarScene->SlowDownTime(); minusPressed = minusCurrent;
        if (rCurrent && !rPressed) solarScene->ResetTime(); rPressed = rCurrent;

        // Effect triggers
        static bool key1, key2, key3, keyC, keySpace;
        bool cur1 = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
        bool cur2 = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
        bool cur3 = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
        bool curC = glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS;
        bool curSpace = glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS;

        if (cur1 && !key1) solarScene->CreateMeteorShower(); key1 = cur1;
        if (cur2 && !key2) solarScene->CreateSpaceDebris(); key2 = cur2;
        if (cur3 && !key3) solarScene->TriggerSolarFlare(); key3 = cur3;
        if (curC && !keyC) solarScene->ToggleCinematicMode(); keyC = curC;
        if (curSpace && !keySpace) solarScene->ClearDynamicObjects(); keySpace = curSpace;

        editor.Update(deltaTime);
        engine.UpdateFrame(deltaTime);
        engine.RenderFrame();
        editor.Render();
        glfwSwapBuffers(window);
    }

    editor.Shutdown();
    return 0;
}
