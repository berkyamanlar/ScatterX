#include "InputManager.h"

// Define static members
LeftMousePicker InputManager::m_leftMouseButton;
int InputManager::m_mouseX = 0;
int InputManager::m_mouseY = 0;

void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        m_leftMouseButton.IsPressed = (action == GLFW_PRESS);
        m_leftMouseButton.x = static_cast<int>(xpos);
        m_leftMouseButton.y = static_cast<int>(ypos);

        //if (action == GLFW_PRESS) {
        //    std::cout << "Mouse Clicked at (" << m_leftMouseButton.x << ", " << m_leftMouseButton.y << ")\n";
        //}
    }
}

void InputManager::UpdateMousePosition(GLFWwindow* window)
{
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    m_mouseX = static_cast<int>(xpos);
    m_mouseY = static_cast<int>(ypos);
}

int InputManager::GetMouseX()
{
    return m_mouseX;
}

int InputManager::GetMouseY()
{
    return m_mouseY;
}
