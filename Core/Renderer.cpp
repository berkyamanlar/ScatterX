#include "Renderer.h"
#include "Mesh.h"

#include <iostream>

Renderer::Renderer():
    camera(1280, 720)
{
    m_pickingTexture.Init(1280, 720);

    setupGridLayout();
    setupCoordinateSystem();
    setupSceneCollection();
    setupPickingShader();
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

void Renderer::setupPickingShader()
{
    // Create a new shader program for picking
    pickingShaderProgram = std::make_unique<Shader>("shaders/picking.vert", "shaders/picking.frag");
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

void Renderer::drawSceneCollection() {
    for (int i = 0; i < sceneCollectionMeshes.size(); ++i) {
        if (sceneCollectionMeshes[i].isVisible) {
            // Activate the mesh's shader program
            sceneCollectionMeshes[i].objectShaderProgram->Activate();

            // Set polygon mode
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            // Update model matrix
            sceneCollectionMeshes[i].UpdateModelMatrix();
            glm::mat4 modelMatrix = sceneCollectionMeshes[i].GetModelMatrix();

            // Pass the model matrix to the shader
            glUniformMatrix4fv(
                glGetUniformLocation(sceneCollectionMeshes[i].objectShaderProgram->ID, "modelMatrix"),
                1, GL_FALSE, glm::value_ptr(modelMatrix)
            );

            // Set the camera matrix
            camera.Matrix(camera.fov, camera.nearPlane, camera.farPlane,
                *sceneCollectionMeshes[i].objectShaderProgram, "camMatrix");

            // Bind the VAO
            glBindVertexArray(sceneCollectionMeshes[i].VAO_obj);

            // Check if this is the picked object
            if (i == pickedObjectID) {
                // Calculate the number of indices and triangles
                size_t numIndices = sceneCollectionMeshes[i].indices.size();
                size_t numTriangles = numIndices / 3;

                // Make sure the picked triangle is valid
                if (pickedTriangleID >= 0 && pickedTriangleID < numTriangles) {
                    // Set highlight uniform to 0 (not highlighted) for non-picked triangles
                    glUniform1i(
                        glGetUniformLocation(sceneCollectionMeshes[i].objectShaderProgram->ID, "isHighlighted"),
                        0
                    );

                    // Draw the triangles before the picked triangle
                    if (pickedTriangleID > 0) {
                        size_t beforeIndices = pickedTriangleID * 3;
                        glDrawElements(GL_TRIANGLES, beforeIndices, GL_UNSIGNED_INT, 0);
                    }

                    // Draw the triangles after the picked triangle
                    if (pickedTriangleID < numTriangles - 1) {
                        size_t startIndex = (pickedTriangleID + 1) * 3;
                        size_t afterIndices = numIndices - startIndex;
                        glDrawElements(GL_TRIANGLES, afterIndices, GL_UNSIGNED_INT,
                            (void*)(startIndex * sizeof(GLuint)));
                    }

                    // Now draw the picked triangle with highlighting
                    glUniform1i(
                        glGetUniformLocation(sceneCollectionMeshes[i].objectShaderProgram->ID, "isHighlighted"),
                        1
                    );
                    glUniform3f(
                        glGetUniformLocation(sceneCollectionMeshes[i].objectShaderProgram->ID, "highlightColor"),
                        0.0f, 1.0f, 0.0f  // Green highlight
                    );

                    // Draw just the picked triangle
                    size_t startIndex = pickedTriangleID * 3;
                    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT,
                        (void*)(startIndex * sizeof(GLuint)));
                }
                else {
                    // Invalid triangle ID, draw the whole object
                    glUniform1i(
                        glGetUniformLocation(sceneCollectionMeshes[i].objectShaderProgram->ID, "isHighlighted"),
                        0
                    );
                    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_INT, 0);
                }
            }
            else {
                // Not the picked object, draw normally
                glUniform1i(
                    glGetUniformLocation(sceneCollectionMeshes[i].objectShaderProgram->ID, "isHighlighted"),
                    0
                );
                glDrawElements(GL_TRIANGLES, sceneCollectionMeshes[i].indices.size(), GL_UNSIGNED_INT, 0);
            }

            glBindVertexArray(0);
        }
    }
}

void Renderer::drawPickingTexture() {
    // Enable writing to the picking texture
    m_pickingTexture.EnableWriting();

    // Clear the buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the picking shader
    pickingShaderProgram->Activate();

    // Set the outputs for the fragment shader - make sure it matches what your FBO supports
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    // Calculate view and projection matrices from the camera
    glm::mat4 viewMatrix = glm::lookAt(camera.Position, camera.Position + camera.Orientation, camera.Up);
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(camera.fov), (float)camera.width / camera.height, camera.nearPlane, camera.farPlane);

    // Set camera view and projection matrices as uniforms
    glUniformMatrix4fv(
        glGetUniformLocation(pickingShaderProgram->ID, "viewMatrix"),
        1, GL_FALSE, glm::value_ptr(viewMatrix)
    );

    glUniformMatrix4fv(
        glGetUniformLocation(pickingShaderProgram->ID, "projectionMatrix"),
        1, GL_FALSE, glm::value_ptr(projectionMatrix)
    );

    // Draw each mesh in the scene collection with a unique object ID
    for (int i = 0; i < sceneCollectionMeshes.size(); ++i) {
        if (sceneCollectionMeshes[i].isVisible) {
            // Set object index (starting from 1, since 0 is background)
            glUniform1ui(
                glGetUniformLocation(pickingShaderProgram->ID, "objectIndex"),
                i + 1
            );

            // Set drawing index (using same index for now)
            glUniform1ui(
                glGetUniformLocation(pickingShaderProgram->ID, "drawIndex"),
                i + 1
            );

            // Update and set the model matrix
            sceneCollectionMeshes[i].UpdateModelMatrix();
            glm::mat4 modelMatrix = sceneCollectionMeshes[i].GetModelMatrix();

            // Pass the model matrix to the shader
            glUniformMatrix4fv(
                glGetUniformLocation(pickingShaderProgram->ID, "modelMatrix"),
                1, GL_FALSE, glm::value_ptr(modelMatrix)
            );

            // Bind the VAO and draw the mesh
            glBindVertexArray(sceneCollectionMeshes[i].VAO_obj);

            // Draw the elements - primitive IDs are automatically assigned
            glDrawElements(GL_TRIANGLES, sceneCollectionMeshes[i].indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    // Disable writing to the picking texture
    m_pickingTexture.DisableWriting();

    // If the mouse is clicked, read the pixel to determine which object was clicked
    if (m_InputManager.m_leftMouseButton.IsPressed) {
        // Get the window size
        int width, height;
        glfwGetWindowSize(glfwGetCurrentContext(), &width, &height);

        // Read the pixel at the mouse position
        PickingTexture::PixelInfo pixel = m_pickingTexture.ReadPixel(
            m_InputManager.m_leftMouseButton.x,
            height - m_InputManager.m_leftMouseButton.y - 1  // Convert to OpenGL coordinates
        );

        // Update the picked object ID and triangle ID
        if (pixel.ObjectID != 0) {
            // Store the previous selections
            int prevObjectID = pickedObjectID;
            int prevTriangleID = pickedTriangleID;

            // Update the current selections
            pickedObjectID = pixel.ObjectID - 1;  // Subtract 1 to get back to 0-based index
            pickedTriangleID = pixel.PrimID;      // Store the primitive (triangle) ID

            // If we have a valid selection, update the triangle's properties
            if (pickedObjectID >= 0 && pickedObjectID < sceneCollectionMeshes.size()) {
                Mesh& mesh = sceneCollectionMeshes[pickedObjectID];

                // Clear previous selection if it exists
                if (prevObjectID >= 0 && prevObjectID < sceneCollectionMeshes.size() &&
                    prevTriangleID >= 0 && prevTriangleID < sceneCollectionMeshes[prevObjectID].triangles.size()) {
                    sceneCollectionMeshes[prevObjectID].SetTriangleSelected(prevTriangleID, false);
                }

                // Set new selection
                if (pickedTriangleID >= 0 && pickedTriangleID < mesh.triangles.size()) {
                    mesh.SetTriangleSelected(pickedTriangleID, true);
                    }
                }
            }
        }
        else {
            // Clear selection if clicking on background
            if (pickedObjectID >= 0 && pickedObjectID < sceneCollectionMeshes.size() &&
                pickedTriangleID >= 0 && pickedTriangleID < sceneCollectionMeshes[pickedObjectID].triangles.size()) {
                sceneCollectionMeshes[pickedObjectID].SetTriangleSelected(pickedTriangleID, false);
            }

            pickedObjectID = -1;  // No object picked
            pickedTriangleID = -1;  // No triangle picked
        }
}
