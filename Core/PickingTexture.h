#pragma once

#include <iostream>
#include <glad/glad.h>

class PickingTexture
{
public:
    PickingTexture() {}

    ~PickingTexture();

    void Init(unsigned int WindowWidth, unsigned int WindowHeight);

    void EnableWriting();

    void DisableWriting();

    struct PixelInfo {
       GLuint ObjectID = 0;
       GLuint DrawID = 0;
       GLuint PrimID = 0;

        void Print()
        {
            printf("Object %d draw %d prim %d\n", ObjectID, DrawID, PrimID);
        }
    };

    PixelInfo ReadPixel(unsigned int x, unsigned int y);

private:
    GLuint m_fbo = 0;
    GLuint m_pickingTexture = 0;
    GLuint m_depthTexture = 0;
};