#ifndef SRC_RENDERER_OBJECTS_OBJECTS_HPP
#define SRC_RENDERER_OBJECTS_OBJECTS_HPP

#include <Types.hpp>

#include "../../Core.hpp"

#include "Renderer/Mesh.hpp"
#include "../../Math/Math.hpp"

#include <Reflection/Reflection.hpp>


struct SphereParams
{
    float radius;
    int   xSegments;
    int   ySegments;
};

NB_REFLECT_STRUCT(
    SphereParams,
    NB_FIELD(
        SphereParams,
        radius
    ),
    NB_FIELD(
        SphereParams,
        xSegments
    ),
    NB_FIELD(
        SphereParams,
        ySegments
    )
)

struct TorusParams
{
    float majorRadius;
    float minorRadius;
    int   xSegments;
    int   ySegments;
};

NB_REFLECT_STRUCT(
    TorusParams,
    NB_FIELD(
        TorusParams,
        majorRadius
    ),
    NB_FIELD(
        TorusParams,
        minorRadius
    ),
    NB_FIELD(
        TorusParams,
        xSegments
    ),
    NB_FIELD(
        TorusParams,
        ySegments
    )
)


struct CylinderParams
{
    float radius;
    float height;
    int   xSegments;
    int   ySegments;
};


NB_REFLECT_STRUCT(
    CylinderParams,
    NB_FIELD(
        CylinderParams,
        radius
    ),
    NB_FIELD(
        CylinderParams,
        height
    ),
    NB_FIELD(
        CylinderParams,
        xSegments
    ),
    NB_FIELD(
        CylinderParams,
        ySegments
    )
)

struct PlaneParams
{
    float width;
    float height;
    int   xSegments;
    int   ySegments;
};

NB_REFLECT_STRUCT(
    PlaneParams,
    NB_FIELD(
        PlaneParams,
        width
    ),
    NB_FIELD(
        PlaneParams,
        height
    ),
    NB_FIELD(
        PlaneParams,
        xSegments
    ),
    NB_FIELD(
        PlaneParams,
        ySegments
    )
)

struct ConeParams
{
    float radius;
    float height;
    int radialSegments;
    int heightSegments;
};

NB_REFLECT_STRUCT(
    ConeParams,
    NB_FIELD(
        ConeParams,
        radius
    ),
    NB_FIELD(
        ConeParams,
        height
    ),
    NB_FIELD(
        ConeParams,
        radialSegments
    ),
    NB_FIELD(
        ConeParams,
        heightSegments
    )
)

struct PyramidParams
{
    float radius;
    float height;
    int sides;
};

NB_REFLECT_STRUCT(
    PyramidParams,
    NB_FIELD(
        PyramidParams,
        radius
    ),
    NB_FIELD(
        PyramidParams,
        height
    ),
    NB_FIELD(
        PyramidParams,
        sides
    )
)


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
            Ref<Mesh> createTorus(
                ParametricSegments segments,
                const float majorRadius,
                const float minorRadius
            ) noexcept;

            Ref<Mesh> createCylinder(
                const float radius,
                const float height,
                uint32      radialSegments,
                uint32      heightSegments
            ) noexcept;

            Ref<Mesh> createPlane(
                const float width,
                const float height,
                uint32      xSegments,
                uint32      ySegments
            ) noexcept;

            Ref<Mesh> createCone(
                const float radius,
                const float height,
                uint32      radialSegments,
                uint32      heightSegments
            ) noexcept;

            Ref<Mesh> createPyramid(
                const float radius,
                const float height,
                uint32      sides
            ) noexcept;
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

//


#endif

