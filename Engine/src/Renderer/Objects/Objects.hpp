#ifndef SRC_RENDERER_OBJECTS_OBJECTS_HPP
#define SRC_RENDERER_OBJECTS_OBJECTS_HPP

#include "../../Core.hpp"

#include "../Mesh.hpp"
#include "../../Math/Math.hpp"

namespace nb
{
    namespace Renderer
    {
        Ref<Mesh> generateTorus(const uint32_t segments, const uint32_t rings, const float majorRadius, const float minorRadius) noexcept;
        Ref<Mesh> generateSphere(const uint32_t segments, const uint32_t rings, const float radius) noexcept;
        Ref<Mesh> generateEllipticalCylinder(const uint32_t segments, const uint32_t a, const uint32_t b, const uint32_t height);
        Ref<Mesh> generateDodecahedron() noexcept;
    };
};

#endif

