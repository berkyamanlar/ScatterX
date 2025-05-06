#version 330 core

// Vertex attributes
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

// Uniforms
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

// Output data to fragment shader
flat out uint out_ObjectIndex;
flat out uint out_DrawIndex;

// Object identifiers
uniform uint objectIndex;
uniform uint drawIndex;

void main()
{
    // Calculate the final position
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);
    
    // Pass the object ID to fragment shader
    out_ObjectIndex = objectIndex;
    out_DrawIndex = drawIndex;
}