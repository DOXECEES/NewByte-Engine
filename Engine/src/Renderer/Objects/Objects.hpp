#ifndef NB_RENDERER_OBJECTS_HPP
#define NB_RENDERER_OBJECTS_HPP

#include <Core.hpp>
#include <Math/Math.hpp>
#include <Reflection/Reflection.hpp>
#include <Renderer/Mesh.hpp>
#include <Types.hpp>
#include <vector>

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
    int   radialSegments;
    int   heightSegments;
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
    int   sides;
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

struct CubeParams
{
    int   size;
};

NB_REFLECT_STRUCT(
    CubeParams,
    NB_FIELD(
        CubeParams,
        size
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
                uint32 u;
                uint32 v;
            };

            Ref<Mesh> createCube(const float size) noexcept;

            Ref<Mesh> createSphere(
                float  radius,
                uint32 xSegments,
                uint32 ySegments
            ) noexcept;

            Ref<Mesh> createTorus(
                ParametricSegments segments,
                float              majorRadius,
                float              minorRadius
            ) noexcept;
            Ref<Mesh> createCylinder(
                float  radius,
                float  height,
                uint32 radialSegments,
                uint32 heightSegments
            ) noexcept;
            Ref<Mesh> createPlane(
                float  width,
                float  height,
                uint32 xSegments,
                uint32 ySegments
            ) noexcept;
            Ref<Mesh> createCone(
                float  radius,
                float  height,
                uint32 radialSegments,
                uint32 heightSegments
            ) noexcept;
            Ref<Mesh> createPyramid(
                float  radius,
                float  height,
                uint32 sides
            ) noexcept;

            void generateGridIndices(
                std::vector<uint32>& indices,
                uint32               xSegments,
                uint32               ySegments
            ) noexcept;
        } // namespace PrimitiveGenerators

        enum class MeshType
        {
            CUBE,
            SPHERE
        };

        struct IPrimitiveObject
        {
            virtual ~IPrimitiveObject()               = default;
            virtual MeshType getType() const noexcept = 0;
        };

        struct Cube : public IPrimitiveObject
        {
            MeshType getType() const noexcept override
            {
                return MeshType::CUBE;
            }
        };

        struct Sphere : public IPrimitiveObject
        {
            MeshType getType() const noexcept override
            {
                return MeshType::SPHERE;
            }
            float radius = 1.0f;
        };

        class PrimitiveMeshFactory
        {
        public:
            static Ref<Mesh> create(const IPrimitiveObject& object) noexcept;
        };
    } // namespace Renderer
} // namespace nb

#endif