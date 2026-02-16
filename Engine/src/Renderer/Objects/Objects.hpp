#ifndef SRC_RENDERER_OBJECTS_OBJECTS_HPP
#define SRC_RENDERER_OBJECTS_OBJECTS_HPP

#include <Types.hpp>

#include "../../Core.hpp"

#include "Renderer/Mesh.hpp"
#include "../../Math/Math.hpp"

namespace nb
{
    namespace Renderer
    {
        namespace PrimitiveGenerators
        {
            struct ParametricSegments
            {
                float u;
                float v;
            };

            Ref<Mesh> createCube() noexcept;
            Ref<Mesh> createSphere(const float radius, uint32 xSegments, uint32 ySegments) noexcept;
            Ref<Mesh> createTorus(const ParametricSegments& segments, const float majorRadius, const float minorRadius) noexcept;
        };

        enum class MeshType
        {
            CUBE,
            SPHERE,
        };

        struct IPrimitiveObject
        {
        public:
            virtual ~IPrimitiveObject();
            virtual MeshType getType() const noexcept;
        };

        struct Cube : public IPrimitiveObject
        {
        public:
            MeshType getType() const noexcept override;
        };

        struct Sphere : public IPrimitiveObject
        {
        public:
            MeshType getType() const noexcept override;
            float radius;
        };


        class PrimitiveMeshFactory
        {
        public:
            static Ref<Mesh> create(const IPrimitiveObject& object) noexcept;

        };

        //Ref<Mesh> generateCube() noexcept;
        Ref<Mesh> generateTorus(const uint32_t segments, const uint32_t rings, const float majorRadius, const float minorRadius) noexcept;
        Ref<Mesh> generateSphere(const uint32_t segments, const uint32_t rings, const float radius) noexcept;
        Ref<Mesh> generateEllipticalCylinder(const uint32_t segments, const uint32_t a, const uint32_t b, const uint32_t height);
        Ref<Mesh> generateDodecahedron() noexcept;
    };
};

#endif

