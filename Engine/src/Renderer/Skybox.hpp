#ifndef SRC_RENDERER_SKYBOX_HPP
#define SRC_RENDERER_SKYBOX_HPP

#include <glad/glad.h>

#include <memory>
#include <vector>
#include <array>
#include <utility>

#include "Shader.hpp"
#include "../Core/EngineSettings.hpp"
#include "Mesh.hpp"

#include "../../dependencies/lodepng/lodepng.h"

#include "../Debug.hpp"

namespace nb
{
    namespace Renderer
    {
        class Skybox
        {
            


            public:
            
                Skybox();
                ~Skybox();
            
                void render(Ref<Renderer::Shader> shader);

            private:
                std::vector<uint8_t> textureData;
                GLuint cubemapTexture;
                Renderer::Mesh *mesh;

                std::vector<Vertex> skyboxVertices = {
                    // Координаты для 8 вершин куба
                    Vertex({-1.0f, -1.0f, -1.0f}, {0, 0, -1}, {1, 1, 1}, {0, 1}), // 0
                    Vertex({1.0f, -1.0f, -1.0f}, {0, 0, -1}, {1, 1, 1}, {1, 1}),  // 1
                    Vertex({1.0f, 1.0f, -1.0f}, {0, 0, -1}, {1, 1, 1}, {1, 0}),   // 2
                    Vertex({-1.0f, 1.0f, -1.0f}, {0, 0, -1}, {1, 1, 1}, {0, 0}),  // 3
                    Vertex({-1.0f, -1.0f, 1.0f}, {0, 0, 1}, {1, 1, 1}, {0, 1}),   // 4
                    Vertex({1.0f, -1.0f, 1.0f}, {0, 0, 1}, {1, 1, 1}, {1, 1}),    // 5
                    Vertex({1.0f, 1.0f, 1.0f}, {0, 0, 1}, {1, 1, 1}, {1, 0}),     // 6
                    Vertex({-1.0f, 1.0f, 1.0f}, {0, 0, 1}, {1, 1, 1}, {0, 0})     // 7
                };

                std::vector<unsigned int> skyboxIndices = {
                    // Передняя сторона
                    0, 1, 2, 0, 2, 3,
                    // Задняя сторона
                    4, 6, 5, 4, 7, 6,
                    // Левая сторона
                    4, 3, 7, 4, 0, 3,
                    // Правая сторона
                    1, 5, 6, 1, 6, 2,
                    // Верхняя сторона
                    3, 2, 6, 3, 6, 7,
                    // Нижняя сторона
                    4, 5, 1, 4, 1, 0};

        };
    };
};

#endif

