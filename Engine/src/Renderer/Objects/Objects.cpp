// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Objects.hpp"

#include "Renderer/Color.hpp"

 namespace nb
 {
     namespace Renderer
     {
         Ref<Mesh> nb::Renderer::PrimitiveGenerators::createCube() noexcept
         {
             static std::vector<Vertex> verticies = {
                 // Позиция                          // Нормаль
                 Vertex({ -1.0f, -1.0f, -1.0f }, { -0.577f, -0.577f, -0.577f }),
                 Vertex({  1.0f, -1.0f, -1.0f }, {  0.577f, -0.577f, -0.577f }),
                 Vertex({  1.0f,  1.0f, -1.0f }, {  0.577f,  0.577f, -0.577f }),
                 Vertex({ -1.0f,  1.0f, -1.0f }, { -0.577f,  0.577f, -0.577f }),
                 Vertex({ -1.0f, -1.0f,  1.0f }, { -0.577f, -0.577f,  0.577f }),
                 Vertex({  1.0f, -1.0f,  1.0f }, {  0.577f, -0.577f,  0.577f }),
                 Vertex({  1.0f,  1.0f,  1.0f }, {  0.577f,  0.577f,  0.577f }),
                 Vertex({ -1.0f,  1.0f,  1.0f }, { -0.577f,  0.577f,  0.577f })
             };

             static std::vector<uint32_t> indicies = {
                 0, 1, 2, 2, 3, 0,
                 // Передняя (+Z)
                 4, 6, 5, 6, 4, 7,
                 // Левая (−X)
                 4, 0, 3, 3, 7, 4,
                 // Правая (+X)
                 1, 5, 6, 6, 2, 1,
                 // Верх (+Y)
                 3, 2, 6, 6, 7, 3,
                 // Низ (−Y)
                 4, 5, 1, 1, 0, 4
             };

             return createRef<Mesh>(
                 verticies,
                 indicies
             );
         }

         Ref<Mesh> PrimitiveGenerators::createSphere(const float radius, uint32 xSegments, uint32 ySegments) noexcept
         {
             // Зададим детализацию по умолчанию, если параметров нет в сигнатуре
            

             std::vector<Vertex> vertices;
             std::vector<uint32> indices;

             const float PI = 3.14159265359f;

             for (uint32 y = 0; y <= ySegments; ++y)
             {
                 float v = (float)y / (float)ySegments;
                 float theta = v * PI; // Угол широты

                 for (uint32 x = 0; x <= xSegments; ++x)
                 {
                     float u = (float)x / (float)xSegments;
                     float phi = u * 2.0f * PI; // Угол долготы

                     // Вычисляем координаты на единичной сфере
                     float xPos = std::cos(phi) * std::sin(theta);
                     float yPos = std::cos(theta); // Y - это "верх"
                     float zPos = std::sin(phi) * std::sin(theta);

                     Math::Vector3<float> pos(xPos * radius, yPos * radius, zPos * radius);
                     Math::Vector3<float> normal(xPos, yPos, zPos);
                     Math::Vector2<float> uv(u, v);

                     // Вычисляем тангент (vec4)
                     // Тангент — это производная позиции по параметру U
                     // Для сферы это вектор, лежащий в горизонтальной плоскости (направление вдоль параллели)
                     Math::Vector4<float> tangent(
                         -std::sin(phi),
                         0.0f,
                         std::cos(phi),
                         1.0f // Направление битангенса
                     );

                     vertices.push_back(Vertex(pos, normal, {1.0f,1.0f,1.0f}, uv, tangent));
                 }
             }

             // Генерация индексов
             for (uint32 y = 0; y < ySegments; ++y)
             {
                 for (uint32 x = 0; x < xSegments; ++x)
                 {
                     uint32 first = y * (xSegments + 1) + x;
                     uint32 second = first + xSegments + 1;

                     indices.push_back(first);
                     indices.push_back(second);
                     indices.push_back(first + 1);

                     indices.push_back(second);
                     indices.push_back(second + 1);
                     indices.push_back(first + 1);
                 }
             }

             return std::make_shared<Mesh>(vertices, indices);
         }

         /*u in[0, 2pi], v in[0, 2pi]
          x = (R + r * cos(v)) * cos(u)
          y = (R + r * cos(v)) * sin(u)
          z = r * sin(v)*/


		 Ref<Mesh> PrimitiveGenerators::createTorus(const ParametricSegments& segments, const float majorRadius, const float minorRadius) noexcept
		 {
			 const int segU = segments.u;
			 const int segV = segments.v;
			 const float R = majorRadius;
			 const float r = minorRadius;

			 const float uStep = 2.0f * nb::Math::Constants::PI / segU;
			 const float vStep = 2.0f * nb::Math::Constants::PI / segV;

			 std::vector<Vertex> vertices;
			 vertices.reserve(segU * segV);

			 for (int i = 0; i < segU; ++i)
			 {
				 float u = i * uStep;
				 float cosU = std::cos(u);
				 float sinU = std::sin(u);

				 for (int j = 0; j < segV; ++j)
				 {
					 float v = j * vStep;
					 float cosV = std::cos(v);
					 float sinV = std::sin(v);

                     Math::Vector3<float> normal = Math::Vector3<float>{ cosU * cosV, sinU * cosV, sinV };
                     normal.normalize();

                     Vertex vertex = {
                         Math::Vector3<float>{
                             (R + r * cosV)* cosU,
                             (R + r * cosV)* sinU,
                             r* sinV
                         },
                         Colors::GOLD.asVec3(),
						 normal,
						 Math::Vector2<float>{ u / (float)(2.0f * nb::Math::Constants::PI), v / (float)(2.0f * nb::Math::Constants::PI) }
					 };

					 vertices.push_back(vertex);
				 }
			 }

			 auto index = [](int i, int j, int segV) {
				 return i * segV + j;
		     };

			 std::vector<uint32_t> indices;
			 indices.reserve(segU * segV * 6);

			 for (int i = 0; i < segU; ++i)
             {
				 for (int j = 0; j < segV; ++j)
                 {
					 int nextI = (i + 1) % segU;
					 int nextJ = (j + 1) % segV;

					 uint32_t i0 = index(i, j, segV);
					 uint32_t i1 = index(nextI, j, segV);
					 uint32_t i2 = index(i, nextJ, segV);
					 uint32_t i3 = index(nextI, nextJ, segV);

					 indices.insert(indices.end(), { i0, i1, i2, i2, i1, i3 });
				 }
			 }

			 return createRef<Mesh>(vertices, indices);
		 }

      

         //Ref<Mesh> generateTorus(const uint32_t segments, const uint32_t rings, const float majorRadius, const float minorRadius) noexcept
         //{
         //    std::vector<Vertex> verticies;
         //    std::vector<uint32_t> indicies;

         //    const float deltaU = 2.0f * Math::Constants::PI / segments;
         //    const float deltaV = 2.0f * Math::Constants::PI / rings;

         //    for (int i = 0; i <= segments; ++i)
         //    {
         //        float u = i * deltaU;
         //        float cosU = cos(u);
         //        float sinU = sin(u);

         //        for (int j = 0; j <= rings; ++j)
         //        {
         //            float v = j * deltaV;
         //            float cosV = cos(v);
         //            float sinV = sin(v);

         //            verticies.push_back({Math::Vector3<float>{
         //                (majorRadius + minorRadius * cosV) * cosU,
         //                (majorRadius + minorRadius * cosV) * sinU,
         //                minorRadius * sinV}});
         //        }
         //    }

         //    for (int i = 0; i < segments; ++i)
         //    {
         //        for (int j = 0; j < rings; ++j)
         //        {
         //            int current = i * (rings + 1) + j;
         //            int next = (i + 1) * (rings + 1) + j;

         //            indicies.push_back(current);
         //            indicies.push_back(next);
         //            indicies.push_back(current + 1);

         //            indicies.push_back(current + 1);
         //            indicies.push_back(next);
         //            indicies.push_back(next + 1);
         //        }
         //    }

         //    return createRef<Mesh>(std::move(verticies), std::move(indicies));
         //}

         //Ref<Mesh> generateSphere(const uint32_t segments, const uint32_t rings, const float radius) noexcept
         //{
         //    std::vector<Vertex> verticies;
         //    std::vector<uint32_t> indicies;
         //    const float deltaU = 2.0f * Math::Constants::PI / segments;
         //    const float deltaV = 2.0f * Math::Constants::PI / rings;
         //    for (int i = 0; i <= segments; ++i)
         //    {
         //        float u = i * deltaU;
         //        float cosU = cos(u);
         //        float sinU = sin(u);

         //        for (int j = 0; j <= rings; ++j)
         //        {
         //            float v = j * deltaV;
         //            float cosV = cos(v);
         //            float sinV = sin(v);

         //            verticies.emplace_back({Math::Vector3<float>{
         //                radius * cosU * cosV,
         //                radius * sinU * cosV,
         //                radius * sinV}});
         //        }
         //    }

         //    for (int i = 0; i < segments; ++i)
         //    {
         //        for (int j = 0; j < rings; ++j)
         //        {
         //            int current = i * (rings + 1) + j;
         //            int next = (i + 1) * (rings + 1) + j;

         //            indicies.push_back(current);
         //            indicies.push_back(next);
         //            indicies.push_back(current + 1);

         //            indicies.push_back(current + 1);
         //            indicies.push_back(next);
         //            indicies.push_back(next + 1);
         //        }
         //    }
         //    return createRef<Mesh>(std::move(verticies), std::move(indicies));
         //}
         //Ref<Mesh> generateEllipticalCylinder(const uint32_t segments, const uint32_t a, const uint32_t b, const uint32_t height)
         //{
         //    const float deltaU = 2.0f * Math::Constants::PI / segments;

         //    std::vector<Vertex> verticies;
         //    std::vector<uint32_t> indicies;
         //    for (int i = 0; i <= segments; ++i)
         //    {
         //        float u = i * deltaU;
         //        float cosU = cos(u);
         //        float sinU = sin(u);

         //        verticies.emplace_back(Math::Vector3<float>{a * cosU, b * sinU, -height / 2.0f});
         //        verticies.emplace_back(Math::Vector3<float>{a * cosU, b * sinU, height / 2.0f});
         //    }

         //    // verticies.push_back(Math::Vector3<float>{0.0f, 0.0f, -height / 2.0f});
         //    // uint32_t bottomCenterIndex = verticies.size() - 1;

         //    // verticies.push_back(Math::Vector3<float>{0.0f, 0.0f, height / 2.0f});
         //    // uint32_t topCenterIndex = verticies.size() - 1;

         //    //Генерация индексов для оснований
         //    // for (int i = 0; i < segments; ++i)
         //    // {
         //    //     indicies.push_back(bottomCenterIndex);
         //    //     indicies.push_back(i * 2);
         //    //     indicies.push_back((i + 1) * 2);

         //    //     indicies.push_back(topCenterIndex);
         //    //     indicies.push_back((i + 1) * 2 + 1);
         //    //     indicies.push_back(i * 2 + 1);
         //    // }

         //    // Генерация индексов для боковой поверхности
         //    for (int i = 0; i < segments; ++i)
         //    {
         //        uint32_t bottom1 = i * 2;        // Нижняя точка текущего сегмента
         //        uint32_t bottom2 = (i + 1) * 2;  // Нижняя точка следующего сегмента
         //        uint32_t top1 = i * 2 + 1;       // Верхняя точка текущего сегмента
         //        uint32_t top2 = (i + 1) * 2 + 1; // Верхняя точка следующего сегмента

         //        // Первый треугольник (нижняя->верхняя->следующая нижняя)
         //        indicies.push_back(bottom1);
         //        indicies.push_back(top1);
         //        indicies.push_back(bottom2);

         //        // Второй треугольник (следующая нижняя->верхняя->следующая верхняя)
         //        indicies.push_back(bottom2);
         //        indicies.push_back(top1);
         //        indicies.push_back(top2);
         //    }

         //    return createRef<Mesh>(std::move(verticies), std::move(indicies));
         //}

         //Ref<Mesh> generateDodecahedron() noexcept
         //{
         //    std::vector<Vertex> vertices {
         //        Math::Vector3<float>(1.0f, 1.0f, 1.0f),
         //        Math::Vector3<float>(1.0f, 1.0f, -1.0f),
         //        Math::Vector3<float>(1.0f, -1.0f, 1.0f),
         //        Math::Vector3<float>(1.0f, -1.0f, -1.0f),
         //        Math::Vector3<float>(-1.0f, 1.0f, 1.0f),
         //        Math::Vector3<float>(-1.0f, 1.0f, -1.0f),
         //        Math::Vector3<float>(-1.0f, -1.0f, 1.0f),
         //        Math::Vector3<float>(-1.0f, -1.0f, -1.0f),
         //        Math::Vector3<float>(0.0f, (1.0f + sqrt(5.0f)) / 2.0f, 0.0f),
         //        Math::Vector3<float>(0.0f, -(1.0f + sqrt(5.0f)) / 2.0f, 0.0f),
         //        Math::Vector3<float>((1.0f + sqrt(5.0f)) / 2.0f, 0.0f, 0.0f),
         //        Math::Vector3<float>(-(1.0f + sqrt(5.0f)) / 2.0f, 0.0f, 0.0f),
         //        Math::Vector3<float>(0.0f, 0.0f, (1.0f + sqrt(5.0f)) / 2.0f),
         //        Math::Vector3<float>(0.0f, 0.0f, -(1.0f + sqrt(5.0f)) / 2.0f)
         //    };

         //    std::vector<unsigned int> indices {
         //        // Face 1
         //        0, 8, 4,
         //        0, 1, 8,
         //        8, 1, 10,
         //        10, 4, 8,
         //        10, 5, 4,
         //       
         //        // Face 2
         //        1, 0, 5,
         //        5, 0, 4,
         //        5, 4, 9,
         //        5, 9, 11,
         //        11, 9, 10,

         //        // Face 3
         //        2, 6, 3,
         //        3, 6, 7,
         //        6, 4, 7,
         //        4, 0, 7,
         //        0, 3, 7,

         //        // Face 4
         //        6, 2, 10,
         //        10, 2, 1,
         //        10, 1, 5,

         //        // Face 5
         //        4, 6, 10,
         //        4, 10, 11,
         //        4, 11, 9,

         //        // Face 6
         //        7, 3, 2,
         //        2, 6, 7,
         //        7, 11, 5,
         //        5, 1, 2,

         //        // Face 7
         //        10, 2, 3,
         //        3, 0, 10,
         //        10, 5, 3,
         //       
         //        // Face 8
         //        1, 0, 3,
         //        3, 5, 1,
         //        5, 11, 3,

         //        // Face 9
         //        7, 6, 10,
         //        10, 1, 7,
         //        1, 5, 10,

         //        // Face 10
         //        8, 9, 4,
         //        4, 0, 8,

         //        // Face 11
         //        0, 5, 9,
         //        9, 10, 5,

         //        // Face 12
         //        10, 11, 9,
         //        9, 8, 10
         //    };

         //    return createRef<Mesh>(std::move(vertices), std::move(indices));
         //}
     };
 };

 