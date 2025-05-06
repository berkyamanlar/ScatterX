#pragma once

#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <GLFW/glfw3.h>
#include <iostream>

// Struct to track left mouse button state and click position
struct LeftMousePicker {
    bool IsPressed = false;
    int x = 0;
    int y = 0;
};

class InputManager {
public:
    // Callback for mouse button events
    static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static LeftMousePicker m_leftMouseButton;

    // Continuously update mouse position (call every frame)
    static void UpdateMousePosition(GLFWwindow* window);

    // Getters for current mouse position
    static int GetMouseX();
    static int GetMouseY();

private:
    static int m_mouseX;
    static int m_mouseY;
};

#endif // INPUT_MANAGER_H
