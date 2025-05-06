#version 330 core

// Outputs colors in RGBA
out vec4 FragColor;

// Inputs the color from the Vertex Shader
in vec3 fragColor;

// Highlight parameters
uniform bool isHighlighted = false;
uniform vec3 highlightColor = vec3(0.0, 1.0, 0.0); // Default to green

void main()
{
    if (isHighlighted) {
        // Use highlight color when the triangle is picked
        FragColor = vec4(highlightColor, 1.0);
    } else {
        // Directly use the color passed from the vertex shader
        FragColor = vec4(fragColor, 1.0);  // Add 1.0 for alpha (opaque)
    }
}