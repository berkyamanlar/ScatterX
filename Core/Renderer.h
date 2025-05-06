#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>

#include "shaderClass.h"
#include "Camera.h"
#include "Mesh.h"
#include "InputManager.h"
#include "PickingTexture.h"

class Renderer {
public:
    Renderer();
    ~Renderer();

    Camera camera;
    InputManager m_InputManager;
    std::vector<Mesh>sceneCollectionMeshes;

    void setupGridLayout();
    void setupSceneCollection();
    void setupCoordinateSystem();
    void setupPickingShader();

    void drawGridLayout();
    void drawCoordinateSystem();
    void drawSceneCollection();
    void drawPickingTexture();

    // Grid
    bool gridNeedsUpdate = false;
    float size = 150.0f;
    float divisions = 150.0f;
    float step;

private:
    // Coordinate system
    std::unique_ptr<Shader> axisShaderProgram;
    GLuint VAO_axis, VBO_axis;

    // Grid
    std::unique_ptr<Shader> gridShaderProgram;
    GLuint VAO_grid, VBO_grid;

    // Picking
    std::unique_ptr<Shader> pickingShaderProgram;
    PickingTexture m_pickingTexture;
    int pickedObjectID = -1;
    int pickedTriangleID = -1;
};
