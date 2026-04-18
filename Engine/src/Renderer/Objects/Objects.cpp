// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "Objects.hpp"
#include <cmath>

namespace nb
{
    namespace Renderer
    {
        namespace
        {
            const Math::Vector3<float> DEFAULT_COLOR{1.0f, 1.0f, 1.0f};
            const float                INV_SQRT_THREE = 0.577350269f;
        } // namespace

        void PrimitiveGenerators::generateGridIndices(
            std::vector<uint32>& indices,
            uint32               xSegments,
            uint32               ySegments
        ) noexcept
        {
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
        }

        Ref<Mesh> PrimitiveGenerators::createCube(const float size) noexcept
        {
            std::vector<Vertex> vertices;
            std::vector<uint32> indices;

            vertices.reserve(24);
            indices.reserve(36);

            const float s = size * 0.5f;

            struct Face
            {
                Math::Vector3<float> normal;
                Math::Vector4<float> tangent;
                Math::Vector3<float> pos[4];
            };

            Face faces[6] = {
                // Front (+Z)
                {{0, 0, 1}, {1, 0, 0, 1}, {{-s, -s, s}, {s, -s, s}, {s, s, s}, {-s, s, s}}},
                // Back (-Z)
                {{0, 0, -1}, {-1, 0, 0, 1}, {{s, -s, -s}, {-s, -s, -s}, {-s, s, -s}, {s, s, -s}}},
                // Left (-X)
                {{-1, 0, 0}, {0, 0, 1, 1}, {{-s, -s, -s}, {-s, -s, s}, {-s, s, s}, {-s, s, -s}}},
                // Right (+X)
                {{1, 0, 0}, {0, 0, -1, 1}, {{s, -s, s}, {s, -s, -s}, {s, s, -s}, {s, s, s}}},
                // Top (+Y)
                {{0, 1, 0}, {1, 0, 0, 1}, {{-s, s, s}, {s, s, s}, {s, s, -s}, {-s, s, -s}}},
                // Bottom (-Y)
                {{0, -1, 0}, {-1, 0, 0, 1}, {{-s, -s, -s}, {s, -s, -s}, {s, -s, s}, {-s, -s, s}}}
            };

            Math::Vector2<float> uvs[4] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
            Math::Vector3<float> color(1.0f, 1.0f, 1.0f);

            for (uint32 i = 0; i < 6; ++i)
            {
                uint32 baseIdx = (uint32)vertices.size();

                for (uint32 v = 0; v < 4; ++v)
                {
                    vertices.emplace_back(
                        faces[i].pos[v], faces[i].normal, color, uvs[v], faces[i].tangent
                    );
                }

                indices.push_back(baseIdx);
                indices.push_back(baseIdx + 1);
                indices.push_back(baseIdx + 2);

                indices.push_back(baseIdx);
                indices.push_back(baseIdx + 2);
                indices.push_back(baseIdx + 3);
            }

            Renderer::Mesh::PrimitiveDescriptor desc = {
                .type = "cube", .parameters = {{"size", size}}
            };

            return std::make_shared<Mesh>(vertices, indices, "", desc);
        }

        Ref<Mesh> PrimitiveGenerators::createSphere(
            const float radius,
            uint32      xSegments,
            uint32      ySegments
        ) noexcept
        {
            if (radius <= 0.0f)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::FATAL, "Sphere radius must be positive")
                    .with("radius", radius);
            }

            std::vector<Vertex> vertices;
            std::vector<uint32> indices;

            const float pi = Math::Constants::PI;

            vertices.reserve((xSegments + 1) * (ySegments + 1));
            indices.reserve(xSegments * ySegments * 6);

            for (uint32 y = 0; y <= ySegments; ++y)
            {
                const float v     = static_cast<float>(y) / static_cast<float>(ySegments);
                const float theta = v * pi;

                for (uint32 x = 0; x <= xSegments; ++x)
                {
                    const float u   = static_cast<float>(x) / static_cast<float>(xSegments);
                    const float phi = u * 2.0f * pi;

                    const float xPos = std::cos(phi) * std::sin(theta);
                    const float yPos = std::cos(theta);
                    const float zPos = std::sin(phi) * std::sin(theta);

                    const Math::Vector3<float> pos(xPos * radius, yPos * radius, zPos * radius);
                    const Math::Vector3<float> normal(xPos, yPos, zPos);
                    const Math::Vector2<float> uv(u, v);
                    const Math::Vector4<float> tangent(-std::sin(phi), 0.0f, std::cos(phi), 1.0f);

                    vertices.emplace_back(pos, normal, DEFAULT_COLOR, uv, tangent);
                }
            }

            generateGridIndices(indices, xSegments, ySegments);

            Mesh::PrimitiveDescriptor desc = {
                .type       = "sphere",
                .parameters = {
                    {"radius", radius},
                    {"xSegments", static_cast<float>(xSegments)},
                    {"ySegments", static_cast<float>(ySegments)}
                }
            };

            return createRef<Mesh>(vertices, indices, "", desc);
        }

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
            const float  pi        = Math::Constants::PI;

            vertices.reserve((xSegments + 1) * (ySegments + 1));
            indices.reserve(xSegments * ySegments * 6);

            for (uint32 y = 0; y <= ySegments; ++y)
            {
                const float v      = static_cast<float>(y) / static_cast<float>(ySegments);
                const float phi    = v * 2.0f * pi;
                const float cosPhi = std::cos(phi);
                const float sinPhi = std::sin(phi);

                for (uint32 x = 0; x <= xSegments; ++x)
                {
                    const float u        = static_cast<float>(x) / static_cast<float>(xSegments);
                    const float theta    = u * 2.0f * pi;
                    const float cosTheta = std::cos(theta);
                    const float sinTheta = std::sin(theta);

                    const float xPos = (majorRadius + minorRadius * cosPhi) * cosTheta;
                    const float yPos = (majorRadius + minorRadius * cosPhi) * sinTheta;
                    const float zPos = minorRadius * sinPhi;

                    const Math::Vector3<float> pos(xPos, yPos, zPos);
                    const Math::Vector3<float> normal(cosPhi * cosTheta, cosPhi * sinTheta, sinPhi);
                    const Math::Vector4<float> tangent(-sinTheta, cosTheta, 0.0f, 1.0f);
                    const Math::Vector2<float> uv(u, v);

                    vertices.emplace_back(pos, normal, DEFAULT_COLOR, uv, tangent);
                }
            }

            generateGridIndices(indices, xSegments, ySegments);

            Mesh::PrimitiveDescriptor desc = {
                .type       = "torus",
                .parameters = {
                    {"majorRadius", majorRadius},
                    {"minorRadius", minorRadius},
                    {"xSegments", static_cast<float>(xSegments)},
                    {"ySegments", static_cast<float>(ySegments)}
                }
            };

            return createRef<Mesh>(vertices, indices, "", desc);
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

            const float pi         = Math::Constants::PI;
            const float halfHeight = height * 0.5f;

            vertices.reserve(
                (radialSegments + 1) * (heightSegments + 1) + (radialSegments + 1) * 2 + 2
            );

            // Тело цилиндра
            for (uint32 y = 0; y <= heightSegments; ++y)
            {
                const float v    = static_cast<float>(y) / static_cast<float>(heightSegments);
                const float yPos = v * height - halfHeight;

                for (uint32 x = 0; x <= radialSegments; ++x)
                {
                    const float u     = static_cast<float>(x) / static_cast<float>(radialSegments);
                    const float theta = u * 2.0f * pi;
                    const float cosTheta = std::cos(theta);
                    const float sinTheta = std::sin(theta);

                    const Math::Vector3<float> pos(radius * cosTheta, yPos, radius * sinTheta);
                    const Math::Vector3<float> normal(cosTheta, 0.0f, sinTheta);
                    const Math::Vector2<float> uv(u, v);
                    const Math::Vector4<float> tangent(-sinTheta, 0.0f, cosTheta, 1.0f);

                    vertices.emplace_back(pos, normal, DEFAULT_COLOR, uv, tangent);
                }
            }
            generateGridIndices(indices, radialSegments, heightSegments);

            // Крышки
            auto addCap = [&](bool isTop)
            {
                const float  yPos      = isTop ? halfHeight : -halfHeight;
                const float  normalY   = isTop ? 1.0f : -1.0f;
                const uint32 centerIdx = static_cast<uint32>(vertices.size());

                vertices.emplace_back(
                    Math::Vector3<float>{0.0f, yPos, 0.0f},
                    Math::Vector3<float>{0.0f, normalY, 0.0f}, DEFAULT_COLOR,
                    Math::Vector2<float>{0.5f, 0.5f}, Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}
                );

                for (uint32 x = 0; x <= radialSegments; ++x)
                {
                    const float theta = (static_cast<float>(x) / radialSegments) * 2.0f * pi;
                    const float cosT  = std::cos(theta);
                    const float sinT  = std::sin(theta);

                    vertices.emplace_back(
                        Math::Vector3<float>{radius * cosT, yPos, radius * sinT},
                        Math::Vector3<float>{0.0f, normalY, 0.0f}, DEFAULT_COLOR,
                        Math::Vector2<float>{cosT * 0.5f + 0.5f, sinT * 0.5f + 0.5f},
                        Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}
                    );

                    if (x < radialSegments)
                    {
                        const uint32 curr = centerIdx + 1 + x;
                        if (isTop)
                        {
                            indices.push_back(centerIdx);
                            indices.push_back(curr);
                            indices.push_back(curr + 1);
                        }
                        else
                        {
                            indices.push_back(centerIdx);
                            indices.push_back(curr + 1);
                            indices.push_back(curr);
                        }
                    }
                }
            };

            addCap(true);
            addCap(false);

            Mesh::PrimitiveDescriptor desc = {
                .type       = "cylinder",
                .parameters = {
                    {"radius", radius},
                    {"height", height},
                    {"xSegments", static_cast<float>(radialSegments)},
                    {"ySegments", static_cast<float>(heightSegments)}
                }
            };

            return createRef<Mesh>(vertices, indices, "", desc);
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
            const float halfWidth  = width * 0.5f;
            const float halfHeight = height * 0.5f;

            for (uint32 y = 0; y <= ySegments; ++y)
            {
                const float v    = static_cast<float>(y) / static_cast<float>(ySegments);
                const float yPos = v * height - halfHeight;

                for (uint32 x = 0; x <= xSegments; ++x)
                {
                    const float u    = static_cast<float>(x) / static_cast<float>(xSegments);
                    const float xPos = u * width - halfWidth;

                    vertices.emplace_back(
                        Math::Vector3<float>{xPos, yPos, 0.0f},
                        Math::Vector3<float>{0.0f, 0.0f, 1.0f}, DEFAULT_COLOR,
                        Math::Vector2<float>{u, v}, Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}
                    );
                }
            }
            generateGridIndices(indices, xSegments, ySegments);

            Mesh::PrimitiveDescriptor desc = {
                .type       = "plane",
                .parameters = {
                    {"width", width},
                    {"height", height},
                    {"xSegments", static_cast<float>(xSegments)},
                    {"ySegments", static_cast<float>(ySegments)}
                }
            };

            return createRef<Mesh>(vertices, indices, "", desc);
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

            const float pi         = Math::Constants::PI;
            const float halfHeight = height * 0.5f;

            for (uint32 y = 0; y <= heightSegments; ++y)
            {
                const float v          = static_cast<float>(y) / static_cast<float>(heightSegments);
                const float yPos       = v * height - halfHeight;
                const float currRadius = radius * (1.0f - v);

                for (uint32 x = 0; x <= radialSegments; ++x)
                {
                    const float u     = static_cast<float>(x) / static_cast<float>(radialSegments);
                    const float theta = u * 2.0f * pi;
                    const float cosT  = std::cos(theta);
                    const float sinT  = std::sin(theta);

                    Math::Vector3<float> normal(cosT, radius / height, sinT);
                    normal.normalize();

                    vertices.emplace_back(
                        Math::Vector3<float>{currRadius * cosT, yPos, currRadius * sinT}, normal,
                        DEFAULT_COLOR, Math::Vector2<float>{u, v},
                        Math::Vector4<float>{-sinT, 0.0f, cosT, 1.0f}
                    );
                }
            }
            generateGridIndices(indices, radialSegments, heightSegments);

            const uint32 baseCenterIdx = static_cast<uint32>(vertices.size());
            vertices.emplace_back(
                Math::Vector3<float>{0.0f, -halfHeight, 0.0f},
                Math::Vector3<float>{0.0f, -1.0f, 0.0f}, DEFAULT_COLOR,
                Math::Vector2<float>{0.5f, 0.5f}, Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}
            );

            for (uint32 x = 0; x <= radialSegments; ++x)
            {
                const float theta = (static_cast<float>(x) / radialSegments) * 2.0f * pi;
                const float cosT  = std::cos(theta);
                const float sinT  = std::sin(theta);

                vertices.emplace_back(
                    Math::Vector3<float>{radius * cosT, -halfHeight, radius * sinT},
                    Math::Vector3<float>{0.0f, -1.0f, 0.0f}, DEFAULT_COLOR,
                    Math::Vector2<float>{cosT * 0.5f + 0.5f, sinT * 0.5f + 0.5f},
                    Math::Vector4<float>{1.0f, 0.0f, 0.0f, 1.0f}
                );

                if (x < radialSegments)
                {
                    const uint32 curr = baseCenterIdx + 1 + x;
                    indices.push_back(baseCenterIdx);
                    indices.push_back(curr + 1);
                    indices.push_back(curr);
                }
            }

            Mesh::PrimitiveDescriptor desc = {
                .type       = "cone",
                .parameters = {
                    {"radius", radius},
                    {"height", height},
                    {"radialSegments", static_cast<float>(radialSegments)},
                    {"heightSegments", static_cast<float>(heightSegments)}
                }
            };

            return createRef<Mesh>(vertices, indices, "", desc);
        }

        Ref<Mesh> PrimitiveGenerators::createPyramid(
            const float radius,
            const float height,
            uint32      sides
        ) noexcept
        {
            if (sides < 3)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::WARNING, "Pyramid sides increased to 3")
                    .with("provided", sides);
                sides = 3;
            }

            std::vector<Vertex> vertices;
            std::vector<uint32> indices;

            const float pi         = Math::Constants::PI;
            const float halfHeight = height * 0.5f;

            for (uint32 i = 0; i < sides; ++i)
            {
                const float theta1 = (static_cast<float>(i) / sides) * 2.0f * pi;
                const float theta2 = (static_cast<float>(i + 1) / sides) * 2.0f * pi;

                const Math::Vector3<float> pApex(0.0f, halfHeight, 0.0f);
                const Math::Vector3<float> pL(
                    radius * std::cos(theta1), -halfHeight, radius * std::sin(theta1)
                );
                const Math::Vector3<float> pR(
                    radius * std::cos(theta2), -halfHeight, radius * std::sin(theta2)
                );

                Math::Vector3<float> normal = Math::cross(pL - pApex, pR - pApex);
                normal.normalize();

                Math::Vector3<float> tangent3D = pR - pL;
                tangent3D.normalize();
                const Math::Vector4<float> tangent(tangent3D.x, tangent3D.y, tangent3D.z, 1.0f);

                const uint32 baseIdx = static_cast<uint32>(vertices.size());
                vertices.emplace_back(
                    pApex, normal, DEFAULT_COLOR, Math::Vector2<float>{0.5f, 1.0f}, tangent
                );
                vertices.emplace_back(
                    pL, normal, DEFAULT_COLOR,
                    Math::Vector2<float>{static_cast<float>(i) / sides, 0.0f}, tangent
                );
                vertices.emplace_back(
                    pR, normal, DEFAULT_COLOR,
                    Math::Vector2<float>{static_cast<float>(i + 1) / sides, 0.0f}, tangent
                );

                indices.push_back(baseIdx);
                indices.push_back(baseIdx + 1);
                indices.push_back(baseIdx + 2);
            }

            const uint32               bottomCenterIdx = static_cast<uint32>(vertices.size());
            const Math::Vector3<float> bottomNormal(0.0f, -1.0f, 0.0f);
            const Math::Vector4<float> bottomTangent(1.0f, 0.0f, 0.0f, 1.0f);

            vertices.emplace_back(
                Math::Vector3<float>{0.0f, -halfHeight, 0.0f}, bottomNormal, DEFAULT_COLOR,
                Math::Vector2<float>{0.5f, 0.5f}, bottomTangent
            );

            for (uint32 i = 0; i <= sides; ++i)
            {
                const float theta = (static_cast<float>(i) / sides) * 2.0f * pi;
                const float cosT  = std::cos(theta);
                const float sinT  = std::sin(theta);

                vertices.emplace_back(
                    Math::Vector3<float>{radius * cosT, -halfHeight, radius * sinT}, bottomNormal,
                    DEFAULT_COLOR, Math::Vector2<float>{cosT * 0.5f + 0.5f, sinT * 0.5f + 0.5f},
                    bottomTangent
                );

                if (i < sides)
                {
                    const uint32 curr = bottomCenterIdx + 1 + i;
                    indices.push_back(bottomCenterIdx);
                    indices.push_back(curr + 1);
                    indices.push_back(curr);
                }
            }

            Mesh::PrimitiveDescriptor desc = {
                .type       = "pyramid",
                .parameters = {
                    {"radius", radius},
                    {"height", height},
                    {"sides", static_cast<float>(sides)}
                }
            };

            return createRef<Mesh>(vertices, indices, "", desc);
        }

        Ref<Mesh> PrimitiveMeshFactory::create(const IPrimitiveObject& object) noexcept
        {
            switch (object.getType())
            {
            case MeshType::CUBE:
                //return PrimitiveGenerators::createCube();
            case MeshType::SPHERE:
            {
                const auto& sphere = static_cast<const Sphere&>(object);
                // Стандартные сегменты для фабрики
                constexpr uint32 DEFAULT_SEGMENTS = 32;
                return PrimitiveGenerators::createSphere(
                    sphere.radius, DEFAULT_SEGMENTS, DEFAULT_SEGMENTS
                );
            }
            default:
                nb::Error::ErrorManager::instance().report(
                    nb::Error::Type::FATAL, "Unknown mesh type in factory"
                );
                return nullptr;
            }
        }
    } // namespace Renderer
} // namespace nb