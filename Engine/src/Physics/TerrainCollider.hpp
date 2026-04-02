#pragma once

#include <algorithm>
#include <cmath>
#include <vector>

#include "Math/Vector3.hpp"
#include "Renderer/Mesh.hpp" // Путь к твоему файлу Mesh

namespace nb::Physics
{
    struct Triangle
    {
        Math::Vector3<float> v0, v1, v2;
    };

    inline bool GetHeightInTriangle(
        const Math::Vector3<float>& v0,
        const Math::Vector3<float>& v1,
        const Math::Vector3<float>& v2,
        float x,
        float z,
        float& outY
    )
    {
        float det = (v1.z - v2.z) * (v0.x - v2.x) + (v2.x - v1.x) * (v0.z - v2.z);
        if (std::abs(det) < 0.00001f)
        {
            return false;
        }

        float l1 = ((v1.z - v2.z) * (x - v2.x) + (v2.x - v1.x) * (z - v2.z)) / det;
        float l2 = ((v2.z - v0.z) * (x - v2.x) + (v0.x - v2.x) * (z - v2.z)) / det;
        float l3 = 1.0f - l1 - l2;

        if (l1 >= 0 && l2 >= 0 && l3 >= 0)
        {
            outY = l1 * v0.y + l2 * v1.y + l3 * v2.y;
            return true;
        }
        return false;
    }

    struct HeightmapCollider
    {
        std::vector<float> heights;
        int rows = 0; 
        int cols = 0; 
        float minX = 0.0f;
        float minZ = 0.0f;
        float cellSize = 0.1f;

        HeightmapCollider() = default;



        HeightmapCollider(
            int c,
            int r,
            float mX,
            float mZ,
            float size
        )
            : cols(c),
              rows(r),
              minX(mX),
              minZ(mZ),
              cellSize(size)
        {
            heights.assign(cols * rows, -1.0e10f);
        }

        float getHeight(
            float worldX,
            float worldZ
        ) const
        {
            float gx = (worldX - minX) / cellSize;
            float gz = (worldZ - minZ) / cellSize;

            int ix = (int)std::floor(gx);
            int iz = (int)std::floor(gz);

            if (ix < 0 || ix >= cols - 1 || iz < 0 || iz >= rows - 1)
            {
                return -1.0e10f;
            }

            float tx = gx - (float)ix;
            float tz = gz - (float)iz;

            float h00 = heights[iz * cols + ix];
            float h10 = heights[iz * cols + (ix + 1)];
            float h01 = heights[(iz + 1) * cols + ix];
            float h11 = heights[(iz + 1) * cols + (ix + 1)];

            if (h00 < -1e9f || h10 < -1e9f || h01 < -1e9f || h11 < -1e9f)
            {
                return -1.0e10f;
            }

            float h_top = h00 + tx * (h10 - h00);
            float h_bottom = h01 + tx * (h11 - h01);
            return h_top + tz * (h_bottom - h_top);
        }


        // В классе HeightmapCollider
        Math::Vector3<float> getNormal(
            float x,
            float z
        ) const
        {
            float eps = 0.1f;
            float hL = getHeight(x - eps, z);
            float hR = getHeight(x + eps, z);
            float hD = getHeight(x, z - eps);
            float hU = getHeight(x, z + eps);

            Math::Vector3<float> normal(hL - hR, 2.0f * eps, hD - hU);
            normal.normalize();
            return normal; 
        }
    };

    // Основная функция запекания, адаптированная под твой класс Mesh
    inline HeightmapCollider BakeMesh(
        const nb::Renderer::Mesh& mesh,
        float cellSize
    )
    {
        const auto& vertices = mesh.getVertices();
        const auto& indices = mesh.getIndices();
        const auto& aabb = mesh.getAabb3d(); 

        float minX = aabb.minPoint.x;
        float maxX = aabb.maxPoint.x;
        float minZ = aabb.minPoint.z;
        float maxZ = aabb.maxPoint.z;

        int cols = (int)std::ceil((maxX - minX) / cellSize) + 1;
        int rows = (int)std::ceil((maxZ - minZ) / cellSize) + 1;

        HeightmapCollider hm(cols, rows, minX, minZ, cellSize);

        for (size_t i = 0; i < indices.size(); i += 3)
        {
            const auto& v0 = vertices[indices[i]].position;
            const auto& v1 = vertices[indices[i + 1]].position;
            const auto& v2 = vertices[indices[i + 2]].position;

            int iMinX = (int)std::floor(((std::min)({v0.x, v1.x, v2.x}) - minX) / cellSize);
            int iMaxX = (int)std::ceil(((std::max)({v0.x, v1.x, v2.x}) - minX) / cellSize);
            int iMinZ = (int)std::floor(((std::min)({v0.z, v1.z, v2.z}) - minZ) / cellSize);
            int iMaxZ = (int)std::ceil(((std::max)({v0.z, v1.z, v2.z}) - minZ) / cellSize);

            iMinX = (std::max)(0, iMinX);
            iMaxX = (std::min)(hm.cols - 1, iMaxX);
            iMinZ = (std::max)(0, iMinZ);
            iMaxZ = (std::min)(hm.rows - 1, iMaxZ);

            for (int zIdx = iMinZ; zIdx <= iMaxZ; ++zIdx)
            {
                for (int xIdx = iMinX; xIdx <= iMaxX; ++xIdx)
                {
                    float worldX = minX + (float)xIdx * cellSize;
                    float worldZ = minZ + (float)zIdx * cellSize;

                    float currentY;
                    if (GetHeightInTriangle(v0, v1, v2, worldX, worldZ, currentY))
                    {
                        float& storedY = hm.heights[zIdx * hm.cols + xIdx];
                        if (currentY > storedY)
                        {
                            storedY = currentY;
                        }
                    }
                }
            }
        }
        return hm;
    }
} // namespace nb::Physics