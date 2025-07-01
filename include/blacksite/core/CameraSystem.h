
#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>

namespace Blacksite {

class InputSystem {
public:
    InputSystem();
    ~InputSystem();

    void Initialize(GLFWwindow* window);
    void Update();

    // Key state queries
    bool IsKeyPressed(int key) const;
    bool IsKeyJustPressed(int key) const;
    bool IsKeyJustReleased(int key) const;

    // Special keys with state tracking
    bool IsF1JustPressed() const;
    bool IsEscapePressed() const;

private:
    GLFWwindow* m_window = nullptr;
    std::unordered_map<int, bool> m_keyStates;
    std::unordered_map<int, bool> m_previousKeyStates;

    void UpdateKeyState(int key);
};

} // namespace BlacksiteE