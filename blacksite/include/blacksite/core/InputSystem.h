#pragma once

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <string>
#include <unordered_map>

namespace Blacksite {

// Clean key enum
enum class Key {
    // Letters
    A = GLFW_KEY_A,
    B = GLFW_KEY_B,
    C = GLFW_KEY_C,
    D = GLFW_KEY_D,
    E = GLFW_KEY_E,
    F = GLFW_KEY_F,
    G = GLFW_KEY_G,
    H = GLFW_KEY_H,
    I = GLFW_KEY_I,
    J = GLFW_KEY_J,
    K = GLFW_KEY_K,
    L = GLFW_KEY_L,
    M = GLFW_KEY_M,
    N = GLFW_KEY_N,
    O = GLFW_KEY_O,
    P = GLFW_KEY_P,
    Q = GLFW_KEY_Q,
    R = GLFW_KEY_R,
    S = GLFW_KEY_S,
    T = GLFW_KEY_T,
    U = GLFW_KEY_U,
    V = GLFW_KEY_V,
    W = GLFW_KEY_W,
    X = GLFW_KEY_X,
    Y = GLFW_KEY_Y,
    Z = GLFW_KEY_Z,

    // Numbers
    Num0 = GLFW_KEY_0,
    Num1 = GLFW_KEY_1,
    Num2 = GLFW_KEY_2,
    Num3 = GLFW_KEY_3,
    Num4 = GLFW_KEY_4,
    Num5 = GLFW_KEY_5,
    Num6 = GLFW_KEY_6,
    Num7 = GLFW_KEY_7,
    Num8 = GLFW_KEY_8,
    Num9 = GLFW_KEY_9,

    // Special keys
    Space = GLFW_KEY_SPACE,
    Enter = GLFW_KEY_ENTER,
    Escape = GLFW_KEY_ESCAPE,
    Tab = GLFW_KEY_TAB,
    Backspace = GLFW_KEY_BACKSPACE,
    Delete = GLFW_KEY_DELETE,

    // Modifiers
    Shift = GLFW_KEY_LEFT_SHIFT,
    Ctrl = GLFW_KEY_LEFT_CONTROL,
    Alt = GLFW_KEY_LEFT_ALT,

    // Arrows
    Left = GLFW_KEY_LEFT,
    Right = GLFW_KEY_RIGHT,
    Up = GLFW_KEY_UP,
    Down = GLFW_KEY_DOWN,

    // Function keys
    F1 = GLFW_KEY_F1,
    F2 = GLFW_KEY_F2,
    F3 = GLFW_KEY_F3,
    F4 = GLFW_KEY_F4,
    F5 = GLFW_KEY_F5,
    F6 = GLFW_KEY_F6,
    F7 = GLFW_KEY_F7,
    F8 = GLFW_KEY_F8,
    F9 = GLFW_KEY_F9,
    F10 = GLFW_KEY_F10,
    F11 = GLFW_KEY_F11,
    F12 = GLFW_KEY_F12
};

enum class Mouse { Left = GLFW_MOUSE_BUTTON_LEFT, Right = GLFW_MOUSE_BUTTON_RIGHT, Middle = GLFW_MOUSE_BUTTON_MIDDLE };

class InputSystem {
  public:
    InputSystem() = default;
    ~InputSystem() = default;

    bool Initialize(GLFWwindow* window);
    void Update();
    void Shutdown();

    // Main API - supports both enum and char
    bool KeyDown(Key key) const { return IsKeyPressed(static_cast<int>(key)); }
    bool KeyDown(char key) const { return KeyDown(CharToKey(key)); }
    bool KeyDown(const std::string& key) const { return KeyDown(key[0]); }

    bool KeyPressed(Key key) const { return IsKeyJustPressed(static_cast<int>(key)); }
    bool KeyPressed(char key) const { return KeyPressed(CharToKey(key)); }

    bool KeyReleased(Key key) const { return IsKeyJustReleased(static_cast<int>(key)); }
    bool KeyReleased(char key) const { return KeyReleased(CharToKey(key)); }

    // Mouse
    bool MouseDown(Mouse button) const { return IsMouseButtonPressed(static_cast<int>(button)); }
    glm::vec2 MousePos() const { return {m_mouseX, m_mouseY}; }
    glm::vec2 MouseDelta() const { return {m_mouseX - m_lastMouseX, m_mouseY - m_lastMouseY}; }

    // Axis system - 2 keys (positive/negative)
    float GetAxis(Key positive, Key negative) const {
        float value = 0.0f;
        if (KeyDown(positive))
            value += 1.0f;
        if (KeyDown(negative))
            value -= 1.0f;
        return value;
    }

    float GetAxis(char positive, char negative) const { return GetAxis(CharToKey(positive), CharToKey(negative)); }

    // 4-key axis system (up/down/left/right)
    glm::vec2 GetAxis2D(Key up, Key down, Key left, Key right) const {
        return glm::vec2(GetAxis(right, left),  // X axis
                         GetAxis(up, down)      // Y axis
        );
    }

    glm::vec2 GetAxis2D(char up, char down, char left, char right) const {
        return GetAxis2D(CharToKey(up), CharToKey(down), CharToKey(left), CharToKey(right));
    }

    // 6-key 3D movement
    glm::vec3 GetAxis3D(Key forward, Key back, Key left, Key right, Key up, Key down) const {
        return glm::vec3(GetAxis(right, left),   // X axis
                         GetAxis(up, down),      // Y axis
                         GetAxis(forward, back)  // Z axis (forward is negative Z)
        );
    }

    glm::vec3 GetAxis3D(char forward, char back, char left, char right, char up, char down) const {
        return GetAxis3D(CharToKey(forward), CharToKey(back), CharToKey(left), CharToKey(right), CharToKey(up),
                         CharToKey(down));
    }

    // Predefined common axes
    float Horizontal() const { return GetAxis('d', 'a'); }  // A/D keys
    float Vertical() const { return GetAxis('w', 's'); }    // W/S keys
    glm::vec2 Movement2D() const { return GetAxis2D('w', 's', 'a', 'd'); }
    glm::vec3 Movement3D() const { return GetAxis3D('w', 's', 'a', 'd', 'e', 'q'); }

    // Speed modifier
    float GetSpeedMultiplier() const {
        if (KeyDown(Key::Shift))
            return 2.0f;  // Fast
        if (KeyDown(Key::Ctrl))
            return 0.3f;  // Slow
        return 1.0f;      // Normal
    }

    // Editor integration
    void SetEditorMode(bool editorActive) { m_editorActive = editorActive; }
    void SetMouseOverUI(bool overUI) { m_mouseOverUI = overUI; }
    bool ShouldCaptureMouse() const { return !m_editorActive || !m_mouseOverUI; }

  private:
    Key CharToKey(char c) const {
        c = std::tolower(c);
        if (c >= 'a' && c <= 'z') {
            return static_cast<Key>(GLFW_KEY_A + (c - 'a'));
        }
        if (c >= '0' && c <= '9') {
            return static_cast<Key>(GLFW_KEY_0 + (c - '0'));
        }
        return Key::Space;  // Default fallback
    }

    bool IsKeyPressed(int key) const;
    bool IsKeyJustPressed(int key) const;
    bool IsKeyJustReleased(int key) const;
    bool IsMouseButtonPressed(int button) const;

    void UpdateKeyStates();
    void UpdateMouseState();

    GLFWwindow* m_window = nullptr;
    bool m_initialized = false;
    bool m_editorActive = false;
    bool m_mouseOverUI = false;
    bool m_firstMouse = true;

    static constexpr int MAX_KEYS = 1024;
    bool m_keys[MAX_KEYS] = {};
    bool m_keysLastFrame[MAX_KEYS] = {};

    double m_mouseX = 0.0, m_mouseY = 0.0;
    double m_lastMouseX = 0.0, m_lastMouseY = 0.0;
};

}  // namespace Blacksite
