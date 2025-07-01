#pragma once

#include "blacksite/graphics/Camera.h"

namespace Blacksite {

class CameraSystem {
public:
    CameraSystem();
    ~CameraSystem();

    void Initialize(float aspectRatio);
    void Update(float deltaTime);

    // Camera controls (delegates to internal camera)
    void SetPosition(const glm::vec3& position);
    void SetTarget(const glm::vec3& target);
    void SetAspectRatio(float aspect);

    // Getters
    glm::vec3 GetPosition() const;
    glm::vec3 GetTarget() const;
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    // Direct camera access for renderer
    Camera& GetCamera() { return m_camera; }
    const Camera& GetCamera() const { return m_camera; }

private:
    Camera m_camera;
    // Future: Add camera movement, animations, etc.
};

} // namespace Blacksite