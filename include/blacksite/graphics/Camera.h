#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Blacksite {

class Camera {
  public:
    Camera();

    // Position and orientation
    void SetPosition(const glm::vec3& position);
    void SetTarget(const glm::vec3& target);  // What we're looking at
    void SetUp(const glm::vec3& up);          // Which way is up (usually Y, but who knows)

    // Projection settings
    void SetPerspective(float fov, float aspect, float nearPlane, float farPlane);
    void SetAspectRatio(float aspect);  // For when the window gets resized

    // Matrix getters
    glm::mat4 GetViewMatrix() const;        // Where the camera is and what it's looking at
    glm::mat4 GetProjectionMatrix() const;  // The perspective transform

    // Getters
    const glm::vec3& GetPosition() const { return m_position; }
    const glm::vec3& GetTarget() const { return m_target; }

  private:
    // Camera position and orientation
    glm::vec3 m_position{0.0f, 0.0f, 3.0f};  // Start a bit back so we can see stuff
    glm::vec3 m_target{0.0f, 0.0f, 0.0f};    // Look at the origin by default
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};        // Y is up

    // Projection parameters
    float m_fov = 45.0f;        // Field of view
    float m_aspect = 1.0f;      // Width/height ratio
    float m_nearPlane = 0.1f;   // Don't make this too small or you'll get Z-fighting
    float m_farPlane = 100.0f;  // Don't make this too big or you'll lose precision
};
}  // namespace Blacksite