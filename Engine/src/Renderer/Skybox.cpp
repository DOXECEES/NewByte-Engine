#include "Skybox.hpp"

namespace nb
{
    namespace Renderer
    {
        Skybox::Skybox()
        {

            uint32_t width = 0;
            uint32_t height = 0;

            const int cols = 4;
            const int rows = 3;

            // for t-like skybox
            std::array<int, 6> ind = {
                6, 4, 1, 9, 5, 7};

            lodepng::decode(textureData, width, height, "C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\res\\skybox.png");

            const int segmentWidth = width / cols;
            const int segmentHeight = height / rows;
            const int segmentSizeInBytes = segmentWidth * segmentHeight * 4;

            uint8_t *xp = new uint8_t[segmentSizeInBytes];
            uint8_t *xn = new uint8_t[segmentSizeInBytes];
            uint8_t *yp = new uint8_t[segmentSizeInBytes];
            uint8_t *yn = new uint8_t[segmentSizeInBytes];
            uint8_t *zp = new uint8_t[segmentSizeInBytes];
            uint8_t *zn = new uint8_t[segmentSizeInBytes];

            std::array<uint8_t *, 6> pl = {
                xp, xn, yp, yn, zp, zn};

            glGenTextures(1, &cubemapTexture);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

            for (int i = 0; i < 6; i++)
            {
                int currentCol = ind[i] % cols;
                int currentRow = (ind[i] - currentCol) % rows;

                int xStart = currentRow * segmentWidth * 4;
                int xEnd = xStart + segmentWidth * 4;

                for (int j = 0; j < segmentHeight; ++j)
                {
                    std::copy(
                        textureData.begin() + ((currentRow * segmentHeight + j) * width + currentCol * segmentWidth) * 4,       
                        textureData.begin() + ((currentRow * segmentHeight + j) * width + (currentCol + 1) * segmentWidth) * 4, 
                        pl[i] + j * segmentWidth * 4                                                                    
                    );
                }

                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, segmentWidth, segmentHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pl[i]);
            }

       


                // std::vector<std::string> faces{
                //     "C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\res\\xpos.png",
                //     "C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\res\\xneg.png",
                //     "C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\res\\ypos.png",
                //     "C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\res\\yneg.png",
                //     "C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\res\\zpos.png",
                //     "C:\\rep\\Hex\\NewByte-Engine\\build\\Engine\\Debug\\res\\zneg.png"
                // };

                // for (unsigned int i = 0; i < faces.size(); i++)
                // {
                //     lodepng::decode(textureData, width, height, faces[i]);

                //     glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData.data());
                //     textureData.clear();
                // }

                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                textureData.clear();

                mesh = new Mesh(skyboxVertices, skyboxIndices);
            }

            Skybox::~Skybox()
            {
                delete mesh;
            }

            void Skybox::render(Ref<Renderer::Shader> shader)
            {
                glDepthFunc(GL_LEQUAL);
                glDepthMask(GL_FALSE);
                glDisable(GL_DEPTH_TEST);
                shader->use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
                mesh->draw(GL_TRIANGLES);
                glDepthFunc(GL_LESS);
                glDepthMask(GL_TRUE);
                glEnable(GL_DEPTH_TEST);
            }
        };
    };
