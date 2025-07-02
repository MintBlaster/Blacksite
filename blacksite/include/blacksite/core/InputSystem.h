#pragma once

struct GLFWwindow;

namespace Blacksite {

class InputSystem {
public:
    InputSystem();
    ~InputSystem();

    bool Initialize(GLFWwindow* window);
    void Shutdown();
    void Update();

    // Key state queries
    bool IsKeyPressed(int key) const;
    bool IsKeyJustPressed(int key) const;
    bool IsKeyJustReleased(int key) const;

    // Specific key queries used by Engine
    bool IsEscapePressed() const;
    bool IsF1JustPressed() const;
    bool IsF5JustPressed() const;

    // Mouse queries
    bool IsMouseButtonPressed(int button) const;
    void GetMousePosition(double& x, double& y) const;
    void GetMouseDelta(double& deltaX, double& deltaY) const;

private:
    GLFWwindow* m_window = nullptr;
    bool m_initialized = false;

    // Key state tracking
    static constexpr int MAX_KEYS = 1024;
    bool m_keys[MAX_KEYS] = {};
    bool m_keysLastFrame[MAX_KEYS] = {};

    // Mouse state
    double m_mouseX = 0.0;
    double m_mouseY = 0.0;
    double m_lastMouseX = 0.0;
    double m_lastMouseY = 0.0;
    bool m_firstMouse = true;

    void UpdateKeyStates();
    void UpdateMouseState();
};

} // namespace Blacksite
