#include "blacksite/core/CameraSystem.h"
#include "blacksite/core/Logger.h"

namespace Blacksite {

CameraSystem::CameraSystem() = default;

CameraSystem::~CameraSystem() = default;

void CameraSystem::Initialize(float aspectRatio) {
    m_camera.SetAspectRatio(aspectRatio);
    m_camera.SetPosition({0, 5, 10});
    m_camera.SetTarget({0, 0, 0});

    BS_INFO(LogCategory::CORE, "CameraSystem: Initialized");
    BS_DEBUG_F(LogCategory::CORE, "CameraSystem: Initial position (%.1f, %.1f, %.1f)",
               GetPosition().x, GetPosition().y, GetPosition().z);
}

void CameraSystem::Update(float deltaTime) {
    (void)deltaTime;
}

void CameraSystem::SetPosition(const glm::vec3& position) {
    m_camera.SetPosition(position);
}

void CameraSystem::SetTarget(const glm::vec3& target) {
    m_camera.SetTarget(target);
}

void CameraSystem::SetAspectRatio(float aspect) {
    m_camera.SetAspectRatio(aspect);
    BS_DEBUG_F(LogCategory::CORE, "CameraSystem: Aspect ratio set to %.2f", aspect);
}

glm::vec3 CameraSystem::GetPosition() const {
    return m_camera.GetPosition();
}

glm::vec3 CameraSystem::GetTarget() const {
    return m_camera.GetTarget();
}

glm::mat4 CameraSystem::GetViewMatrix() const {
    return m_camera.GetViewMatrix();
}

glm::mat4 CameraSystem::GetProjectionMatrix() const {
    return m_camera.GetProjectionMatrix();
}

} // namespace Blacksite
