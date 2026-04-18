// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Objects.hpp"

#include <Color.hpp>

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
                 indicies,
                 ""
             );
         }

         Ref<Mesh> PrimitiveGenerators::createSphere(
             const float radius,
             uint32      xSegments,
             uint32      ySegments
         ) noexcept
         {
             std::vector<Vertex> vertices;
             std::vector<uint32> indices;

             const float PI = 3.14159265359f;

             vertices.reserve((xSegments + 1) * (ySegments + 1));
             indices.reserve(xSegments * ySegments * 6);

             for (uint32 y = 0; y <= ySegments; ++y)
             {
                 float v     = (float)y / (float)ySegments;
                 float theta = v * PI; 

                 for (uint32 x = 0; x <= xSegments; ++x)
                 {
                     float u   = (float)x / (float)xSegments;
                     float phi = u * 2.0f * PI; 

                     float xPos = std::cos(phi) * std::sin(theta);
                     float yPos = std::cos(theta); 
                     float zPos = std::sin(phi) * std::sin(theta);

                     Math::Vector3<float> pos(xPos * radius, yPos * radius, zPos * radius);
                     Math::Vector3<float> normal(xPos, yPos, zPos);
                     Math::Vector2<float> uv(u, v);

                     Math::Vector4<float> tangent(-std::sin(phi), 0.0f, std::cos(phi), 1.0f);

                     vertices.emplace_back(
                         pos, normal, Math::Vector3<float>{1.0f, 1.0f, 1.0f}, uv, tangent
                     );
                 }
             }

             for (uint32 y = 0; y < ySegments; ++y)
             {
                 for (uint32 x = 0; x < xSegments; ++x)
                 {
                     uint32 first  = y * (xSegments + 1) + x;
                     uint32 second = first + xSegments + 1;

                     indices.push_back(first);
                     indices.push_back(second);
                     indices.push_back(first + 1);

                     indices.push_back(second);
                     indices.push_back(second + 1);
                     indices.push_back(first + 1);
                 }
             }

             Renderer::Mesh::PrimitiveDescriptor desc = {
                 .type       = "sphere",
                 .parameters = {
                     {"radius", radius},
                     {"xSegments", (float)xSegments}, 
                     {"ySegments", (float)ySegments}
                 }
             };

             return std::make_shared<Mesh>(vertices, indices, "", desc);
         }

         /*u in[0, 2pi], v in[0, 2pi]
          x = (R + r * cos(v)) * cos(u)
          y = (R + r * cos(v)) * sin(u)
          z = r * sin(v)*/


		 Ref<Mesh> PrimitiveGenerators::createTorus(
             ParametricSegments segments,
             const float        majorRadius,
             const float        minorRadius
         ) noexcept
         {
             std::vector<Vertex> vertices;
             std::vector<uint32> indices;

             const uint32 xSegments = segments.u;
             const uint32 ySegments = segments.v;
             const float  PI        = 3.14159265359f;

             vertices.reserve((xSegments + 1) * (ySegments + 1));
             indices.reserve(xSegments * ySegments * 6);

             for (uint32 y = 0; y <= ySegments; ++y)
             {
                 float v      = (float)y / (float)ySegments;
                 float phi    = v * 2.0f * PI; 
                 float cosPhi = std::cos(phi);
                 float sinPhi = std::sin(phi);

                 for (uint32 x = 0; x <= xSegments; ++x)
                 {
                     float u        = (float)x / (float)xSegments;
                     float theta    = u * 2.0f * PI; 
                     float cosTheta = std::cos(theta);
                     float sinTheta = std::sin(theta);

                     float xPos = (majorRadius + minorRadius * cosPhi) * cosTheta;
                     float yPos = (majorRadius + minorRadius * cosPhi) * sinTheta;
                     float zPos = minorRadius * sinPhi;

                     Math::Vector3<float> pos(xPos, yPos, zPos);

                     Math::Vector3<float> normal(cosPhi * cosTheta, cosPhi * sinTheta, sinPhi);

                     Math::Vector4<float> tangent(-sinTheta, cosTheta, 0.0f, 1.0f);

                     Math::Vector2<float> uv(u, v);

                     vertices.emplace_back(
                         pos, normal, Math::Vector3<float>{1.0f, 1.0f, 1.0f}, uv, tangent
                     );
                 }
             }

             for (uint32 y = 0; y < ySegments; ++y)
             {
                 for (uint32 x = 0; x < xSegments; ++x)
                 {
                     uint32 first  = y * (xSegments + 1) + x;
                     uint32 second = first + xSegments + 1;

                     indices.push_back(first);
                     indices.push_back(second);
                     indices.push_back(first + 1);

                     indices.push_back(second);
                     indices.push_back(second + 1);
                     indices.push_back(first + 1);
                 }
             }

             Renderer::Mesh::PrimitiveDescriptor desc = {
                 .type       = "torus",
                 .parameters = {
                     {"majorRadius", majorRadius},
                     {"minorRadius", minorRadius},
                     {"xSegments", (float)xSegments},
                     {"ySegments", (float)ySegments}
                 }
             };

             return std::make_shared<Mesh>(vertices, indices, "", desc);
         }


         Ref<Mesh> PrimitiveGenerators::createCylinder(
             const float radius,
             const float height,
             uint32      radialSegments,
             uint32      heightSegments
         ) noexcept
         {
             std::vector<Vertex> vertices;
             std::vector<uint32> indices;

             const float PI         = 3.14159265359f;
             const float halfHeight = height * 0.5f;

             vertices.reserve((radialSegments + 1) * (heightSegments + 3) + 2);
             indices.reserve(radialSegments * heightSegments * 6 + radialSegments * 6);

             for (uint32 y = 0; y <= heightSegments; ++y)
             {
                 float v    = (float)y / (float)heightSegments;
                 float yPos = v * height - halfHeight; 

                 for (uint32 x = 0; x <= radialSegments; ++x)
                 {
                     float u     = (float)x / (float)radialSegments;
                     float theta = u * 2.0f * PI;

                     float cosTheta = std::cos(theta);
                     float sinTheta = std::sin(theta);

                     Math::Vector3<float> pos(radius * cosTheta, yPos, radius * sinTheta);
                     Math::Vector3<float> normal(cosTheta, 0.0f, sinTheta);
                     Math::Vector2<float> uv(u, v);
                     Math::Vector4<float> tangent(-sinTheta, 0.0f, cosTheta, 1.0f);

                     vertices.emplace_back(
                         pos, normal, Math::Vector3<float>{1.0f, 1.0f, 1.0f}, uv, tangent
                     );
                 }
             }

             for (uint32 y = 0; y < heightSegments; ++y)
             {
                 for (uint32 x = 0; x < radialSegments; ++x)
                 {
                     uint32 first  = y * (radialSegments + 1) + x;
                     uint32 second = first + radialSegments + 1;

                     indices.push_back(first);
                     indices.push_back(second);
                     indices.push_back(first + 1);

                     indices.push_back(second);
                     indices.push_back(second + 1);
                     indices.push_back(first + 1);
                 }
             }

             uint32 baseVertexIndex = (uint32)vertices.size();

             uint32 topCenterIndex = (uint32)vertices.size();
             vertices.emplace_back(
                 Math::Vector3<float>{0.0f, halfHeight, 0.0f}, // pos
                 Math::Vector3<float>{0.0f, 1.0f, 0.0f},       // normal
                 Math::Vector3<float>{1.0f, 1.0f, 1.0f},       // color
                 Math::Vector2<float>{0.5f, 0.5f},             // uv
                 Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}  // tangent
             );

             for (uint32 x = 0; x <= radialSegments; ++x)
             {
                 float u        = (float)x / (float)radialSegments;
                 float theta    = u * 2.0f * PI;
                 float cosTheta = std::cos(theta);
                 float sinTheta = std::sin(theta);

                 vertices.emplace_back(
                     Math::Vector3<float>{radius * cosTheta, halfHeight, radius * sinTheta},
                     Math::Vector3<float>{0.0f, 1.0f, 0.0f}, Math::Vector3<float>{1.0f, 1.0f, 1.0f},
                     Math::Vector2<float>{cosTheta * 0.5f + 0.5f, sinTheta * 0.5f + 0.5f},
                     Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}
                 );

                 if (x < radialSegments)
                 {
                     uint32 current = topCenterIndex + 1 + x;
                     indices.push_back(topCenterIndex);
                     indices.push_back(current);
                     indices.push_back(current + 1);
                 }
             }

             uint32 bottomCenterIndex = (uint32)vertices.size();
             vertices.emplace_back(
                 Math::Vector3<float>{0.0f, -halfHeight, 0.0f},
                 Math::Vector3<float>{0.0f, -1.0f, 0.0f}, Math::Vector3<float>{1.0f, 1.0f, 1.0f},
                 Math::Vector2<float>{0.5f, 0.5f}, Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}
             );

             for (uint32 x = 0; x <= radialSegments; ++x)
             {
                 float u        = (float)x / (float)radialSegments;
                 float theta    = u * 2.0f * PI;
                 float cosTheta = std::cos(theta);
                 float sinTheta = std::sin(theta);

                 vertices.emplace_back(
                     Math::Vector3<float>{radius * cosTheta, -halfHeight, radius * sinTheta},
                     Math::Vector3<float>{0.0f, -1.0f, 0.0f},
                     Math::Vector3<float>{1.0f, 1.0f, 1.0f},
                     Math::Vector2<float>{cosTheta * 0.5f + 0.5f, sinTheta * 0.5f + 0.5f},
                     Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}
                 );

                 if (x < radialSegments)
                 {
                     uint32 current = bottomCenterIndex + 1 + x;
                     indices.push_back(bottomCenterIndex);
                     indices.push_back(current + 1);
                     indices.push_back(current);
                 }
             }

             Renderer::Mesh::PrimitiveDescriptor desc = {
                 .type       = "cylinder",
                 .parameters = {
                     {"radius", radius},
                     {"height", height},
                     {"xSegments", (float)radialSegments},
                     {"ySegments", (float)heightSegments}
                 }
             };

             return std::make_shared<Mesh>(vertices, indices, "", desc);
         }
      
         Ref<Mesh> PrimitiveGenerators::createPlane(
             const float width,
             const float height,
             uint32      xSegments,
             uint32      ySegments
         ) noexcept
         {
             std::vector<Vertex> vertices;
             std::vector<uint32> indices;

             vertices.reserve((xSegments + 1) * (ySegments + 1));
             indices.reserve(xSegments * ySegments * 6);

             const float halfWidth  = width * 0.5f;
             const float halfHeight = height * 0.5f;

             for (uint32 y = 0; y <= ySegments; ++y)
             {
                 float v = (float)y / (float)ySegments;
                 float yPos = v * height - halfHeight;

                 for (uint32 x = 0; x <= xSegments; ++x)
                 {
                     float u = (float)x / (float)xSegments;
                     float xPos = u * width - halfWidth;

                     Math::Vector3<float> pos(xPos, yPos, 0.0f);

                     Math::Vector3<float> normal(0.0f, 0.0f, 1.0f);

                     Math::Vector4<float> tangent(1.0f, 0.0f, 0.0f, 1.0f);

                     Math::Vector2<float> uv(u, v);

                     vertices.emplace_back(
                         pos, normal, Math::Vector3<float>{1.0f, 1.0f, 1.0f}, uv, tangent
                     );
                 }
             }

             for (uint32 y = 0; y < ySegments; ++y)
             {
                 for (uint32 x = 0; x < xSegments; ++x)
                 {
                     uint32 first  = y * (xSegments + 1) + x;
                     uint32 second = first + xSegments + 1;

                     indices.push_back(first);
                     indices.push_back(second);
                     indices.push_back(first + 1);

                     indices.push_back(second);
                     indices.push_back(second + 1);
                     indices.push_back(first + 1);
                 }
             }

             Renderer::Mesh::PrimitiveDescriptor desc = {
                 .type       = "plane",
                 .parameters = {
                     {"width", width},
                     {"height", height},
                     {"xSegments", (float)xSegments},
                     {"ySegments", (float)ySegments}
                 }
             };

             return std::make_shared<Mesh>(vertices, indices, "", desc);
         }

         Ref<Mesh> PrimitiveGenerators::createCone(
             const float radius,
             const float height,
             uint32      radialSegments,
             uint32      heightSegments
         ) noexcept
         {
             std::vector<Vertex> vertices;
             std::vector<uint32> indices;

             const float PI         = 3.14159265359f;
             const float halfHeight = height * 0.5f;

             vertices.reserve((radialSegments + 1) * (heightSegments + 1) + (radialSegments + 2));
             indices.reserve(radialSegments * heightSegments * 6 + radialSegments * 3);

             for (uint32 y = 0; y <= heightSegments; ++y)
             {
                 float v = (float)y / (float)heightSegments;
                 float yPos = v * height - halfHeight;

                 float currentRadius = radius * (1.0f - v);

                 for (uint32 x = 0; x <= radialSegments; ++x)
                 {
                     float u     = (float)x / (float)radialSegments;
                     float theta = u * 2.0f * PI;

                     float cosTheta = std::cos(theta);
                     float sinTheta = std::sin(theta);

                     Math::Vector3<float> pos(
                         currentRadius * cosTheta, yPos, currentRadius * sinTheta
                     );

                     float                slope = radius / height;
                     Math::Vector3<float> normal(cosTheta, slope, sinTheta);
                     normal.normalize(); 

                     Math::Vector2<float> uv(u, v);
                     Math::Vector4<float> tangent(-sinTheta, 0.0f, cosTheta, 1.0f);

                     vertices.emplace_back(
                         pos, normal, Math::Vector3<float>{1.0f, 1.0f, 1.0f}, uv, tangent
                     );
                 }
             }

             for (uint32 y = 0; y < heightSegments; ++y)
             {
                 for (uint32 x = 0; x < radialSegments; ++x)
                 {
                     uint32 first  = y * (radialSegments + 1) + x;
                     uint32 second = first + radialSegments + 1;

                     indices.push_back(first);
                     indices.push_back(second);
                     indices.push_back(first + 1);

                     indices.push_back(second);
                     indices.push_back(second + 1);
                     indices.push_back(first + 1);
                 }
             }

             uint32 bottomCenterIndex = (uint32)vertices.size();

             vertices.emplace_back(
                 Math::Vector3<float>{0.0f, -halfHeight, 0.0f},
                 Math::Vector3<float>{0.0f, -1.0f, 0.0f}, Math::Vector3<float>{1.0f, 1.0f, 1.0f},
                 Math::Vector2<float>{0.5f, 0.5f}, Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}
             );

             for (uint32 x = 0; x <= radialSegments; ++x)
             {
                 float u        = (float)x / (float)radialSegments;
                 float theta    = u * 2.0f * PI;
                 float cosTheta = std::cos(theta);
                 float sinTheta = std::sin(theta);

                 vertices.emplace_back(
                     Math::Vector3<float>{radius * cosTheta, -halfHeight, radius * sinTheta},
                     Math::Vector3<float>{0.0f, -1.0f, 0.0f},
                     Math::Vector3<float>{1.0f, 1.0f, 1.0f},
                     Math::Vector2<float>{cosTheta * 0.5f + 0.5f, sinTheta * 0.5f + 0.5f},
                     Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}
                 );

                 if (x < radialSegments)
                 {
                     uint32 current = bottomCenterIndex + 1 + x;
                     indices.push_back(bottomCenterIndex);
                     indices.push_back(current + 1);
                     indices.push_back(current);
                 }
             }

             Renderer::Mesh::PrimitiveDescriptor desc = {
                 .type       = "cone",
                 .parameters = {
                     {"radius", radius},
                     {"height", height},
                     {"radialSegments", (float)radialSegments},
                     {"heightSegments", (float)heightSegments}
                 }
             };

             return std::make_shared<Mesh>(vertices, indices, "", desc);
         }


         Ref<Mesh> PrimitiveGenerators::createPyramid(
             const float radius,
             const float height,
             uint32      sides
         ) noexcept
         {
             if (sides < 3)
             {
                 sides = 3;
             }

             std::vector<Vertex> vertices;
             std::vector<uint32> indices;

             const float PI         = 3.14159265359f;
             const float halfHeight = height * 0.5f;

             vertices.reserve(sides * 3 + sides + 2);
             indices.reserve(sides * 3 + sides * 3);

             // --- 1. Боковые грани (Sides) ---
             for (uint32 i = 0; i < sides; ++i)
             {
                 float theta1 = (float)i / (float)sides * 2.0f * PI;
                 float theta2 = (float)(i + 1) / (float)sides * 2.0f * PI;

                 Math::Vector3<float> pApex(0.0f, halfHeight, 0.0f);
                 Math::Vector3<float> pL(
                     radius * std::cos(theta1), -halfHeight, radius * std::sin(theta1)
                 );
                 Math::Vector3<float> pR(
                     radius * std::cos(theta2), -halfHeight, radius * std::sin(theta2)
                 );

                 // Нормаль грани (векторное произведение ребер)
                 Math::Vector3<float> edge1 = pL - pApex;
                 Math::Vector3<float> edge2 = pR - pApex;
                 Math::Vector3<float> normal = Math::cross(edge1, edge2);
                 normal.normalize();

                 // Тангент грани (направлен от левой вершины основания к правой)
                 // Он лежит в плоскости грани и перпендикулярен нормали
                 Math::Vector3<float> tangent3D = (pR - pL);
                 tangent3D.normalize();
                 Math::Vector4<float> tangent(tangent3D.x, tangent3D.y, tangent3D.z, 1.0f);

                 uint32 baseIdx = (uint32)vertices.size();

                 // UV координаты:
                 // Apex - по центру сверху
                 // pL - слева внизу
                 // pR - справа внизу
                 Math::Vector2<float> uvApex(0.5f, 1.0f);
                 Math::Vector2<float> uvL((float)i / sides, 0.0f);
                 Math::Vector2<float> uvR((float)(i + 1) / sides, 0.0f);

                 // Добавляем 3 вершины для треугольной грани
                 vertices.emplace_back(
                     pApex, normal, Math::Vector3<float>{1, 1, 1}, uvApex, tangent
                 );
                 vertices.emplace_back(pL, normal, Math::Vector3<float>{1, 1, 1}, uvL, tangent);
                 vertices.emplace_back(pR, normal, Math::Vector3<float>{1, 1, 1}, uvR, tangent);

                 indices.push_back(baseIdx);
                 indices.push_back(baseIdx + 1);
                 indices.push_back(baseIdx + 2);
             }

             // --- 2. Нижнее основание (Bottom Cap) ---
             uint32               bottomCenterIndex = (uint32)vertices.size();
             Math::Vector3<float> bottomNormal(0.0f, -1.0f, 0.0f);
             Math::Vector4<float> bottomTangent(1.0f, 0.0f, 0.0f, 1.0f); // Направлен вдоль оси X

             // Центр дна
             vertices.emplace_back(
                 Math::Vector3<float>{0.0f, -halfHeight, 0.0f}, bottomNormal,
                 Math::Vector3<float>{1.0f, 1.0f, 1.0f}, Math::Vector2<float>{0.5f, 0.5f},
                 bottomTangent
             );

             // Вершины по кругу для дна
             for (uint32 i = 0; i <= sides; ++i)
             {
                 float theta = (float)i / (float)sides * 2.0f * PI;
                 float cosT  = std::cos(theta);
                 float sinT  = std::sin(theta);

                 vertices.emplace_back(
                     Math::Vector3<float>{radius * cosT, -halfHeight, radius * sinT}, bottomNormal,
                     Math::Vector3<float>{1.0f, 1.0f, 1.0f},
                     Math::Vector2<float>{cosT * 0.5f + 0.5f, sinT * 0.5f + 0.5f}, bottomTangent
                 );

                 if (i < sides)
                 {
                     uint32 current = bottomCenterIndex + 1 + i;
                     indices.push_back(bottomCenterIndex);
                     indices.push_back(current + 1);
                     indices.push_back(current);
                 }
             }

             Renderer::Mesh::PrimitiveDescriptor desc = {
                 .type       = "pyramid",
                 .parameters = {{"radius", radius}, {"height", height}, {"sides", (float)sides}}
             };

             return std::make_shared<Mesh>(vertices, indices, "", desc);
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

 