#include "blacksite/graphics/Camera.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/trigonometric.hpp>

namespace Blacksite {

Camera::Camera() {
    // Default perspective
    SetPerspective(45, 1.0f, 0.1f, 100.f);
}

void Camera::SetPosition(const glm::vec3& position) {
    m_position = position;
    // Note: View matrix will be recalculated on next GetViewMatrix() call
}

void Camera::SetTarget(const glm::vec3& target) {
    m_target = target;
}

void Camera::SetUp(const glm::vec3& up) {
    m_up = up;
    // Usually (0, 1, 0) but sometimes you need a different "up" direction
}

void Camera::SetPerspective(float fov, float aspect, float nearPlane, float farPlane) {
    m_fov = fov;
    m_aspect = aspect;
    m_nearPlane = nearPlane;
    m_farPlane = farPlane;
}

void Camera::SetAspectRatio(float aspect) {
    m_aspect = aspect;
}

glm::mat4 Camera::GetViewMatrix() const {
    // Creates a view matrix that transforms world space to camera space
    return glm::lookAt(m_position, m_target, m_up);
}

glm::mat4 Camera::GetProjectionMatrix() const {
    // Convert our FOV from degrees to radians
    return glm::perspective(glm::radians(m_fov), m_aspect, m_nearPlane, m_farPlane);
}
}  // namespace Blacksite