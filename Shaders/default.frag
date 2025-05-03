#version 330 core

// Outputs colors in RGBA
out vec4 FragColor;

// Inputs the color from the Vertex Shader
in vec3 color;

void main()
{
    // Directly use the color passed from the vertex shader
    FragColor = vec4(color, 1.0);  // Add 1.0 for alpha (opaque)
}
