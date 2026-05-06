#include "Placeholder.hpp"

#include "Math/Vector3.hpp"

namespace nb::OpenGl
{
    GLuint64 createPlaceholderForEmission() noexcept
    {
        static GLuint texID = 0;
        static GLuint64 handle = 0;
        if (texID != 0)
        {
            return handle;
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &texID);

        glTextureStorage2D(texID, 1, GL_RGBA8, 1, 1);

        unsigned char data[] = {0, 0, 0, 255};
        glTextureSubImage2D(texID, 0, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

        glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(texID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        handle = glGetTextureHandleARB(texID);
        glMakeTextureHandleResidentARB(handle);

        return handle;
    }

    GLuint64 createPlaceholderForNoise() noexcept
    {
        static GLuint   texID  = 0;
        static GLuint64 handle = 0;
        if (texID != 0)
        {
            return handle;
        }

        nb::Math::Vector3<float> noiseValues[16];
        for (int i = 0; i < 16; ++i)
        {
            noiseValues[i] = {
                (float)rand() / RAND_MAX * 2.0f - 1.0f, (float)rand() / RAND_MAX * 2.0f - 1.0f,
                0.0f 
            };
        }

        glCreateTextures(GL_TEXTURE_2D, 1, &texID);

        glTextureStorage2D(texID, 1, GL_RGB16F, 4, 4);

        glTextureSubImage2D(texID, 0, 0, 0, 4, 4, GL_RGB, GL_FLOAT, noiseValues);

        glTextureParameteri(texID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(texID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(texID, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(texID, GL_TEXTURE_WRAP_T, GL_REPEAT);

        handle = glGetTextureHandleARB(texID);
        glMakeTextureHandleResidentARB(handle);

        return handle;

    }



}