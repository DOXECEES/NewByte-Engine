#include "TerrainCollider.hpp"


namespace nb::Physics
{

    bool HeightmapCollider::isValidHeight(float height) const noexcept
    {
        return height > INVALID_HEIGHT;
    }
    
    NB_NODISCARD float HeightmapCollider::getHeight(
        float worldX,
        float worldZ
    ) const noexcept
    {
        const float localX = (worldX - originX) / cellSize;
        const float localZ = (worldZ - originZ) / cellSize;

        const int cellIndexX = static_cast<int>(std::floor(localX));
        const int cellIndexZ = static_cast<int>(std::floor(localZ));

        if (cellIndexX < 0 || cellIndexX >= columnCount - 1 || cellIndexZ < 0 ||
            cellIndexZ >= rowCount - 1)
        {
            return INVALID_HEIGHT;
        }

        const float fractionX = localX - static_cast<float>(cellIndexX);
        const float fractionZ = localZ - static_cast<float>(cellIndexZ);

        const float heightBottomLeft = heights[cellIndexZ * columnCount + cellIndexX];
        const float heightBottomRight = heights[cellIndexZ * columnCount + (cellIndexX + 1)];
        const float heightTopLeft = heights[(cellIndexZ + 1) * columnCount + cellIndexX];
        const float heightTopRight = heights[(cellIndexZ + 1) * columnCount + (cellIndexX + 1)];

        if (heightBottomLeft <= INVALID_HEIGHT + 1.0f ||
            heightBottomRight <= INVALID_HEIGHT + 1.0f || heightTopLeft <= INVALID_HEIGHT + 1.0f ||
            heightTopRight <= INVALID_HEIGHT + 1.0f)
        {
            return INVALID_HEIGHT;
        }

        const float lerpXBottom =
            heightBottomLeft + fractionX * (heightBottomRight - heightBottomLeft);
        const float lerpXTop = heightTopLeft + fractionX * (heightTopRight - heightTopLeft);

        return lerpXBottom + fractionZ * (lerpXTop - lerpXBottom);
    }

    NB_NODISCARD Math::Vector3<float> HeightmapCollider::getNormal(
        float worldX,
        float worldZ
    ) const noexcept
    {
        const float step = cellSize * 0.5f;

        const float heightLeft = getHeight(worldX - step, worldZ);
        const float heightRight = getHeight(worldX + step, worldZ);
        const float heightDown = getHeight(worldX, worldZ - step);
        const float heightUp = getHeight(worldX, worldZ + step);

        if (heightLeft <= INVALID_HEIGHT + 1.0f || heightRight <= INVALID_HEIGHT + 1.0f)
        {
            return {0.0f, 1.0f, 0.0f};
        }

        Math::Vector3<float> normal(heightLeft - heightRight, 2.0f * step, heightDown - heightUp);
        normal.normalize();
        return normal;
    }

    bool getHeightInTriangle(
        const Math::Vector3<float>& vertexA,
        const Math::Vector3<float>& vertexB,
        const Math::Vector3<float>& vertexC,
        float                       worldX,
        float                       worldZ,
        float&                      outHeightY
    ) noexcept
    {
        const float determinant = (vertexB.z - vertexC.z) * (vertexA.x - vertexC.x) +
                                  (vertexC.x - vertexB.x) * (vertexA.z - vertexC.z);

        if (std::abs(determinant) < KINDA_SMALL_NUMBER)
        {
            return false;
        }

        const float inverseDeterminant = 1.0f / determinant;

        const float weightA = ((vertexB.z - vertexC.z) * (worldX - vertexC.x) +
                               (vertexC.x - vertexB.x) * (worldZ - vertexC.z)) *
                              inverseDeterminant;
        const float weightB = ((vertexC.z - vertexA.z) * (worldX - vertexC.x) +
                               (vertexA.x - vertexC.x) * (worldZ - vertexC.z)) *
                              inverseDeterminant;
        const float weightC = 1.0f - weightA - weightB;

        if (weightA >= -KINDA_SMALL_NUMBER && weightB >= -KINDA_SMALL_NUMBER &&
            weightC >= -KINDA_SMALL_NUMBER)
        {
            outHeightY = weightA * vertexA.y + weightB * vertexB.y + weightC * vertexC.y;
            return true;
        }
        return false;
    }
    HeightmapCollider bakeMesh(
        const nb::Renderer::Mesh& mesh,
        float                     cellSize
    ) noexcept
    {
        const auto& vertices = mesh.getVertices();
        const auto& indices = mesh.getIndices();
        const auto& bounds = mesh.getAabb3d();

        if (indices.empty() || cellSize <= 0.0f)
        {
            return {};
        }

        const int cols =
            static_cast<int>(std::ceil((bounds.maxPoint.x - bounds.minPoint.x) / cellSize)) + 1;
        const int rows =
            static_cast<int>(std::ceil((bounds.maxPoint.z - bounds.minPoint.z) / cellSize)) + 1;

        HeightmapCollider collider(cols, rows, bounds.minPoint.x, bounds.minPoint.z, cellSize);

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            const auto& vA = vertices[indices[i]].position;
            const auto& vB = vertices[indices[i + 1]].position;
            const auto& vC = vertices[indices[i + 2]].position;

            auto toIndexX = [&](float x)
            {
                return static_cast<int>((x - collider.originX) / cellSize);
            };
            auto toIndexZ = [&](float z)
            {
                return static_cast<int>((z - collider.originZ) / cellSize);
            };

            const int startX = std::max(0, toIndexX(std::min({vA.x, vB.x, vC.x})));
            const int endX =
                std::min(collider.columnCount - 1, toIndexX(std::max({vA.x, vB.x, vC.x})) + 1);
            const int startZ = std::max(0, toIndexZ(std::min({vA.z, vB.z, vC.z})));
            const int endZ =
                std::min(collider.rowCount - 1, toIndexZ(std::max({vA.z, vB.z, vC.z})) + 1);

            for (int zIdx = startZ; zIdx <= endZ; ++zIdx)
            {
                const float worldZ = collider.originZ + static_cast<float>(zIdx) * cellSize;

                for (int xIdx = startX; xIdx <= endX; ++xIdx)
                {
                    const float worldX = collider.originX + static_cast<float>(xIdx) * cellSize;

                    float foundHeightY;
                    if (getHeightInTriangle(vA, vB, vC, worldX, worldZ, foundHeightY))
                    {
                        float& currentStoredHeight =
                            collider
                                .heights[static_cast<size_t>(zIdx) * collider.columnCount + xIdx];

                        if (foundHeightY > currentStoredHeight)
                        {
                            currentStoredHeight = foundHeightY;
                        }
                    }
                }
            }
        }

        collider.sourcePath = const_cast<nb::Renderer::Mesh&>(mesh).getPath();

        return collider;
    }
}; // namespace nb::Physics
