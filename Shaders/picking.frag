#version 330 core

// Input from vertex shader
flat in uint out_ObjectIndex;
flat in uint out_DrawIndex;

// Output data - single RGB output where:
// R = ObjectID, G = DrawID, B = PrimID
layout(location = 0) out uvec4 FragColor;

void main()
{
    // Pack all IDs into a single color output
    // gl_PrimitiveID is built-in and gives the current primitive index
    FragColor = uvec4(out_ObjectIndex, out_DrawIndex, gl_PrimitiveID, 0);
}