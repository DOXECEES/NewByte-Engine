// #include "Objects.hpp"

// namespace nb
// {
//     namespace Renderer
//     {
//         Ref<Mesh> generateTorus(const uint32_t segments, const uint32_t rings, const float majorRadius, const float minorRadius) noexcept
//         {
//             std::vector<Vertex> verticies;
//             std::vector<uint32_t> indicies;

//             const float deltaU = 2.0f * Math::Constants::PI / segments;
//             const float deltaV = 2.0f * Math::Constants::PI / rings;

//             for (int i = 0; i <= segments; ++i)
//             {
//                 float u = i * deltaU;
//                 float cosU = cos(u);
//                 float sinU = sin(u);

//                 for (int j = 0; j <= rings; ++j)
//                 {
//                     float v = j * deltaV;
//                     float cosV = cos(v);
//                     float sinV = sin(v);

//                     verticies.push_back({Math::Vector3<float>{
//                         (majorRadius + minorRadius * cosV) * cosU,
//                         (majorRadius + minorRadius * cosV) * sinU,
//                         minorRadius * sinV}});
//                 }
//             }

//             for (int i = 0; i < segments; ++i)
//             {
//                 for (int j = 0; j < rings; ++j)
//                 {
//                     int current = i * (rings + 1) + j;
//                     int next = (i + 1) * (rings + 1) + j;

//                     indicies.push_back(current);
//                     indicies.push_back(next);
//                     indicies.push_back(current + 1);

//                     indicies.push_back(current + 1);
//                     indicies.push_back(next);
//                     indicies.push_back(next + 1);
//                 }
//             }

//             return createRef<Mesh>(std::move(verticies), std::move(indicies));
//         }

//         Ref<Mesh> generateSphere(const uint32_t segments, const uint32_t rings, const float radius) noexcept
//         {
//             std::vector<Vertex> verticies;
//             std::vector<uint32_t> indicies;
//             const float deltaU = 2.0f * Math::Constants::PI / segments;
//             const float deltaV = 2.0f * Math::Constants::PI / rings;
//             for (int i = 0; i <= segments; ++i)
//             {
//                 float u = i * deltaU;
//                 float cosU = cos(u);
//                 float sinU = sin(u);

//                 for (int j = 0; j <= rings; ++j)
//                 {
//                     float v = j * deltaV;
//                     float cosV = cos(v);
//                     float sinV = sin(v);

//                     verticies.push_back({Math::Vector3<float>{
//                         radius * cosU * cosV,
//                         radius * sinU * cosV,
//                         radius * sinV}});
//                 }
//             }

//             for (int i = 0; i < segments; ++i)
//             {
//                 for (int j = 0; j < rings; ++j)
//                 {
//                     int current = i * (rings + 1) + j;
//                     int next = (i + 1) * (rings + 1) + j;

//                     indicies.push_back(current);
//                     indicies.push_back(next);
//                     indicies.push_back(current + 1);

//                     indicies.push_back(current + 1);
//                     indicies.push_back(next);
//                     indicies.push_back(next + 1);
//                 }
//             }
//             return createRef<Mesh>(std::move(verticies), std::move(indicies));
//         }
//         Ref<Mesh> generateEllipticalCylinder(const uint32_t segments, const uint32_t a, const uint32_t b, const uint32_t height)
//         {
//             const float deltaU = 2.0f * Math::Constants::PI / segments;

//             std::vector<Vertex> verticies;
//             std::vector<uint32_t> indicies;
//             for (int i = 0; i <= segments; ++i)
//             {
//                 float u = i * deltaU;
//                 float cosU = cos(u);
//                 float sinU = sin(u);

//                 verticies.push_back(Math::Vector3<float>{a * cosU, b * sinU, -height / 2.0f});
//                 verticies.push_back(Math::Vector3<float>{a * cosU, b * sinU, height / 2.0f});
//             }

//             // verticies.push_back(Math::Vector3<float>{0.0f, 0.0f, -height / 2.0f});
//             // uint32_t bottomCenterIndex = verticies.size() - 1;

//             // verticies.push_back(Math::Vector3<float>{0.0f, 0.0f, height / 2.0f});
//             // uint32_t topCenterIndex = verticies.size() - 1;

//             //Генерация индексов для оснований
//             // for (int i = 0; i < segments; ++i)
//             // {
//             //     indicies.push_back(bottomCenterIndex);
//             //     indicies.push_back(i * 2);
//             //     indicies.push_back((i + 1) * 2);

//             //     indicies.push_back(topCenterIndex);
//             //     indicies.push_back((i + 1) * 2 + 1);
//             //     indicies.push_back(i * 2 + 1);
//             // }

//             // Генерация индексов для боковой поверхности
//             for (int i = 0; i < segments; ++i)
//             {
//                 uint32_t bottom1 = i * 2;        // Нижняя точка текущего сегмента
//                 uint32_t bottom2 = (i + 1) * 2;  // Нижняя точка следующего сегмента
//                 uint32_t top1 = i * 2 + 1;       // Верхняя точка текущего сегмента
//                 uint32_t top2 = (i + 1) * 2 + 1; // Верхняя точка следующего сегмента

//                 // Первый треугольник (нижняя->верхняя->следующая нижняя)
//                 indicies.push_back(bottom1);
//                 indicies.push_back(top1);
//                 indicies.push_back(bottom2);

//                 // Второй треугольник (следующая нижняя->верхняя->следующая верхняя)
//                 indicies.push_back(bottom2);
//                 indicies.push_back(top1);
//                 indicies.push_back(top2);
//             }

//             return createRef<Mesh>(std::move(verticies), std::move(indicies));
//         }

//         Ref<Mesh> generateDodecahedron() noexcept
//         {
//             std::vector<Vertex> vertices {
//                 Math::Vector3<float>(1.0f, 1.0f, 1.0f),
//                 Math::Vector3<float>(1.0f, 1.0f, -1.0f),
//                 Math::Vector3<float>(1.0f, -1.0f, 1.0f),
//                 Math::Vector3<float>(1.0f, -1.0f, -1.0f),
//                 Math::Vector3<float>(-1.0f, 1.0f, 1.0f),
//                 Math::Vector3<float>(-1.0f, 1.0f, -1.0f),
//                 Math::Vector3<float>(-1.0f, -1.0f, 1.0f),
//                 Math::Vector3<float>(-1.0f, -1.0f, -1.0f),
//                 Math::Vector3<float>(0.0f, (1.0f + sqrt(5.0f)) / 2.0f, 0.0f),
//                 Math::Vector3<float>(0.0f, -(1.0f + sqrt(5.0f)) / 2.0f, 0.0f),
//                 Math::Vector3<float>((1.0f + sqrt(5.0f)) / 2.0f, 0.0f, 0.0f),
//                 Math::Vector3<float>(-(1.0f + sqrt(5.0f)) / 2.0f, 0.0f, 0.0f),
//                 Math::Vector3<float>(0.0f, 0.0f, (1.0f + sqrt(5.0f)) / 2.0f),
//                 Math::Vector3<float>(0.0f, 0.0f, -(1.0f + sqrt(5.0f)) / 2.0f)
//             };

//             std::vector<unsigned int> indices {
//                 // Face 1
//                 0, 8, 4,
//                 0, 1, 8,
//                 8, 1, 10,
//                 10, 4, 8,
//                 10, 5, 4,
                
//                 // Face 2
//                 1, 0, 5,
//                 5, 0, 4,
//                 5, 4, 9,
//                 5, 9, 11,
//                 11, 9, 10,

//                 // Face 3
//                 2, 6, 3,
//                 3, 6, 7,
//                 6, 4, 7,
//                 4, 0, 7,
//                 0, 3, 7,

//                 // Face 4
//                 6, 2, 10,
//                 10, 2, 1,
//                 10, 1, 5,

//                 // Face 5
//                 4, 6, 10,
//                 4, 10, 11,
//                 4, 11, 9,

//                 // Face 6
//                 7, 3, 2,
//                 2, 6, 7,
//                 7, 11, 5,
//                 5, 1, 2,

//                 // Face 7
//                 10, 2, 3,
//                 3, 0, 10,
//                 10, 5, 3,
                
//                 // Face 8
//                 1, 0, 3,
//                 3, 5, 1,
//                 5, 11, 3,

//                 // Face 9
//                 7, 6, 10,
//                 10, 1, 7,
//                 1, 5, 10,

//                 // Face 10
//                 8, 9, 4,
//                 4, 0, 8,

//                 // Face 11
//                 0, 5, 9,
//                 9, 10, 5,

//                 // Face 12
//                 10, 11, 9,
//                 9, 8, 10
//             };

//             return createRef<Mesh>(std::move(vertices), std::move(indices));
//         }
//     };
// };
