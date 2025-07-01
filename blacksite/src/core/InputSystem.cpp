#include "blacksite/core/InputSystem.h"
#include "blacksite/core/Logger.h"
#include <GLFW/glfw3.h>
#include <cstring>

namespace Blacksite {

InputSystem::InputSystem() {
    // Initialize key arrays
    std::memset(m_keys, false, sizeof(m_keys));
    std::memset(m_keysLastFrame, false, sizeof(m_keysLastFrame));
}

InputSystem::~InputSystem() {
    Shutdown();
}

bool InputSystem::Initialize(GLFWwindow* window) {
    if (m_initialized) {
        BS_WARN(LogCategory::INPUT, "InputSystem already initialized");
        return true;
    }

    if (!window) {
        BS_ERROR(LogCategory::INPUT, "Cannot initialize InputSystem with null window");
        return false;
    }

    m_window = window;
    m_initialized = true;

    BS_INFO(LogCategory::INPUT, "InputSystem initialized successfully");
    return true;
}

void InputSystem::Shutdown() {
    if (!m_initialized) return;

    m_window = nullptr;
    m_initialized = false;

    BS_INFO(LogCategory::INPUT, "InputSystem shut down");
}

void InputSystem::Update() {
    if (!m_initialized || !m_window) return;

    UpdateKeyStates();
    UpdateMouseState();
}

void InputSystem::UpdateKeyStates() {
    // Copy current frame to last frame
    std::memcpy(m_keysLastFrame, m_keys, sizeof(m_keys));

    // Update current frame key states
    for (int i = 0; i < MAX_KEYS; ++i) {
        m_keys[i] = (glfwGetKey(m_window, i) == GLFW_PRESS);
    }
}

void InputSystem::UpdateMouseState() {
    m_lastMouseX = m_mouseX;
    m_lastMouseY = m_mouseY;

    glfwGetCursorPos(m_window, &m_mouseX, &m_mouseY);

    if (m_firstMouse) {
        m_lastMouseX = m_mouseX;
        m_lastMouseY = m_mouseY;
        m_firstMouse = false;
    }
}

bool InputSystem::IsKeyPressed(int key) const {
    if (key < 0 || key >= MAX_KEYS) return false;
    return m_keys[key];
}

bool InputSystem::IsKeyJustPressed(int key) const {
    if (key < 0 || key >= MAX_KEYS) return false;
    return m_keys[key] && !m_keysLastFrame[key];
}

bool InputSystem::IsKeyJustReleased(int key) const {
    if (key < 0 || key >= MAX_KEYS) return false;
    return !m_keys[key] && m_keysLastFrame[key];
}

bool InputSystem::IsEscapePressed() const {
    return IsKeyPressed(GLFW_KEY_ESCAPE);
}

bool InputSystem::IsF1JustPressed() const {
    return IsKeyJustPressed(GLFW_KEY_F1);
}

bool InputSystem::IsMouseButtonPressed(int button) const {
    if (!m_window) return false;
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void InputSystem::GetMousePosition(double& x, double& y) const {
    x = m_mouseX;
    y = m_mouseY;
}

void InputSystem::GetMouseDelta(double& deltaX, double& deltaY) const {
    deltaX = m_mouseX - m_lastMouseX;
    deltaY = m_mouseY - m_lastMouseY;
}

} // namespace Blacksite
