#ifndef SRC_PHYSICS_TERRAIN_COLLIDER_HPP
#define SRC_PHYSICS_TERRAIN_COLLIDER_HPP

#include <NbCore.hpp>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

#include "Math/Vector3.hpp"
#include "Renderer/Mesh.hpp"

namespace nb::Physics
{
    inline constexpr float INVALID_HEIGHT     = -1.0e10f;
    inline constexpr float KINDA_SMALL_NUMBER = 1e-5f;

    struct Triangle
    {
        Math::Vector3<float> pointA;
        Math::Vector3<float> pointB;
        Math::Vector3<float> pointC;
    };

    bool getHeightInTriangle(
        const Math::Vector3<float>& vertexA,
        const Math::Vector3<float>& vertexB,
        const Math::Vector3<float>& vertexC,
        float                       worldX,
        float                       worldZ,
        float&                      outHeightY
    ) noexcept;

    struct HeightmapCollider
    {
        std::vector<float> heights;
        int                columnCount = 0;
        int                rowCount    = 0;
        float              originX     = 0.0f;
        float              originZ     = 0.0f;
        float              cellSize    = 1.0f;

        HeightmapCollider() = default;

        HeightmapCollider(
            int   cols,
            int   rows,
            float minX,
            float minZ,
            float spacing
        ) noexcept
            : heights(
                  static_cast<size_t>(cols) * rows,
                  INVALID_HEIGHT
              )
            , columnCount(cols)
            , rowCount(rows)
            , originX(minX)
            , originZ(minZ)
            , cellSize(spacing)
        {
        }

        NB_NODISCARD float getHeight(
            float worldX,
            float worldZ
        ) const noexcept;

        NB_NODISCARD Math::Vector3<float> getNormal(
            float worldX,
            float worldZ
        ) const noexcept;
    };

    HeightmapCollider bakeMesh(
        const nb::Renderer::Mesh& mesh,
        float                     cellSize
    ) noexcept;
} // namespace nb::Physics

#endif