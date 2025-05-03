#version 330 core

// Input vertex data
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

// Output data to fragment shader
out vec3 color;

// Transformation matrices
uniform mat4 modelMatrix = mat4(1.0);   // Model's transformation matrix
uniform mat4 camMatrix;    // Camera matrix (view * projection)

void main()
{
    // Apply the model and camera transformations
    gl_Position = camMatrix * modelMatrix * vec4(aPos, 1.0);
    
    // Pass the color to the fragment shader
    color = aColor;
}