#include "blacksite/core/InputSystem.h"
#include "blacksite/core/Logger.h"
#include <cstring>
#include <cctype>

namespace Blacksite {

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

    // Initialize arrays
    std::memset(m_keys, false, sizeof(m_keys));
    std::memset(m_keysLastFrame, false, sizeof(m_keysLastFrame));

    BS_INFO(LogCategory::INPUT, "InputSystem initialized with new API");
    return true;
}

void InputSystem::Update() {
    if (!m_initialized || !m_window) return;

    UpdateKeyStates();

    // Only update mouse if we should capture it
    if (ShouldCaptureMouse()) {
        UpdateMouseState();
    }
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

bool InputSystem::IsMouseButtonPressed(int button) const {
    if (!m_window) return false;
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

void InputSystem::Shutdown() {
    if (!m_initialized) return;

    m_window = nullptr;
    m_initialized = false;

    BS_INFO(LogCategory::INPUT, "InputSystem shut down");
}

} // namespace Blacksite
