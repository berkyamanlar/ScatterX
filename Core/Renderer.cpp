#include "Renderer.h"
#include "Mesh.h"

#include <iostream>

Renderer::Renderer():
    camera(1280, 720)
{
    setupGridLayout();
    setupCoordinateSystem();
    setupSceneCollection();
}

Renderer::~Renderer() {
    // Clean grid layout
    glDeleteVertexArrays(1, &VAO_grid);
    glDeleteBuffers(1, &VBO_grid);

    // Clean coordinate system
    glDeleteVertexArrays(1, &VAO_axis);
    glDeleteBuffers(1, &VBO_axis);
}

void Renderer::setupGridLayout()
{
    float half = size / 2.0f;
    step = size / divisions; // Step size is the side length of the grid squares

    std::vector<float> vertices;

    float r = 0.4f, g = 0.4f, b = 0.4f;

    for (int i = 0; i <= divisions; ++i) {
        float k = -half + i * step;

        // vertical line (constant X, varies Z)
        vertices.insert(vertices.end(), { k, 0.0f, -half, r, g, b });
        vertices.insert(vertices.end(), { k, 0.0f,  half, r, g, b });

        // horizontal line (constant Z, varies X)
        vertices.insert(vertices.end(), { -half, 0.0f, k, r, g, b });
        vertices.insert(vertices.end(), { half, 0.0f, k, r, g, b });
    }

    glGenVertexArrays(1, &VAO_grid);
    glGenBuffers(1, &VBO_grid);
    glBindVertexArray(VAO_grid);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_grid);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    gridShaderProgram = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
}

void Renderer::setupCoordinateSystem() {
    // Define vertices for coordinate axes: X (red), Y (green), Z (blue)
    GLfloat vertices[] = {
        // X-axis
        0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        size / 2, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,
        // Y-axis
        0.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,
        0.0f, size / 2, 0.0f,  0.0f, 1.0f, 0.0f,
        // Z-axis
        0.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, size / 2,  0.0f, 0.0f, 1.0f
    };

    // Generate and bind VAO and VBO
    glGenVertexArrays(1, &VAO_axis);
    glGenBuffers(1, &VBO_axis);

    glBindVertexArray(VAO_axis);
    glBindBuffer(GL_ARRAY_BUFFER, VBO_axis);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Load the shader for axes
    axisShaderProgram = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
}

void Renderer::setupSceneCollection() // This is where we pass layout inputs
{
    for (int i = 0; i < this->sceneCollectionMeshes.size(); ++i)
    {
        // Generate and bind VAO and VBO
        glGenVertexArrays(1, &sceneCollectionMeshes[i].VAO_obj);
        glGenBuffers(1, &sceneCollectionMeshes[i].VBO_obj);
        glGenBuffers(1, &sceneCollectionMeshes[i].EBO_obj); // Create an Element Buffer Object for indices

        glBindVertexArray(sceneCollectionMeshes[i].VAO_obj);
        // Bind and set vertex buffer data
        glBindBuffer(GL_ARRAY_BUFFER, sceneCollectionMeshes[i].VBO_obj);
        glBufferData(GL_ARRAY_BUFFER, sceneCollectionMeshes[i].vertices.size() * sizeof(Vertex), sceneCollectionMeshes[i].vertices.data(), GL_STATIC_DRAW);
        // Bind and set index buffer data
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sceneCollectionMeshes[i].EBO_obj);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sceneCollectionMeshes[i].indices.size() * sizeof(GLuint), sceneCollectionMeshes[i].indices.data(), GL_STATIC_DRAW);

        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        glEnableVertexAttribArray(0);

        // Color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0); // Unbind VAO

        // Load the shader for object
        sceneCollectionMeshes[i].objectShaderProgram = std::make_unique<Shader>("shaders/default.vert", "shaders/default.frag");
        
        // Initialize the model matrix for this mesh
        sceneCollectionMeshes[i].UpdateModelMatrix();
    }
}

void Renderer::drawGridLayout()
{
    if (gridNeedsUpdate) {
        glDeleteBuffers(1, &VBO_grid);
        glDeleteVertexArrays(1, &VAO_grid);
        setupGridLayout();
        setupCoordinateSystem();
        gridNeedsUpdate = false;
    }

    gridShaderProgram->Activate();

    // Create an identity model matrix (no transformations for grid system)
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Pass the model matrix to the shader
    glUniformMatrix4fv(
        glGetUniformLocation(axisShaderProgram->ID, "modelMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(modelMatrix)
    );

    camera.Matrix(camera.fov, camera.nearPlane, camera.farPlane, *gridShaderProgram, "camMatrix");

    glBindVertexArray(VAO_grid);
    glDrawArrays(GL_LINES, 0, (divisions + 1) * 4);
    glBindVertexArray(0);
}

void Renderer::drawCoordinateSystem() {
    // Activate the shader program
    axisShaderProgram->Activate();

    // Create an identity model matrix (no transformations for coordinate system)
    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // Pass the model matrix to the shader
    glUniformMatrix4fv(
        glGetUniformLocation(axisShaderProgram->ID, "modelMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(modelMatrix)
    );

    // Set the camera matrix (view and projection)
    camera.Matrix(camera.fov, camera.nearPlane, camera.farPlane, *axisShaderProgram, "camMatrix");

    // Bind VAO and draw lines
    glBindVertexArray(VAO_axis);
    glDrawArrays(GL_LINES, 0, 6);
    glBindVertexArray(0);
}

void Renderer::drawSceneCollection() // This is where we pass uniform inputs and do model transformations
{
    for (int i = 0; i < sceneCollectionMeshes.size(); ++i)
    {
        if (sceneCollectionMeshes[i].isVisible)
        {
            // Activate the mesh's shader program
            sceneCollectionMeshes[i].objectShaderProgram->Activate();

            // Set polygon mode (wireframe)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            // Make sure the model matrix is updated with the latest transforms
            sceneCollectionMeshes[i].UpdateModelMatrix();

            // Get the model matrix
            glm::mat4 modelMatrix = sceneCollectionMeshes[i].GetModelMatrix();

            // Pass the model matrix to the shader
            glUniformMatrix4fv(
                glGetUniformLocation(sceneCollectionMeshes[i].objectShaderProgram->ID, "modelMatrix"),
                1,
                GL_FALSE,
                glm::value_ptr(modelMatrix)
            );

            // Set the camera matrix (view and projection)
            camera.Matrix(camera.fov, camera.nearPlane, camera.farPlane, *sceneCollectionMeshes[i].objectShaderProgram, "camMatrix");

            // Bind the VAO and draw the mesh using indices
            glBindVertexArray(sceneCollectionMeshes[i].VAO_obj);
            glDrawElements(GL_TRIANGLES, sceneCollectionMeshes[i].indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);

            // Reset polygon mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}
