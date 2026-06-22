#include "JoltDebugRenderer.hpp" // Затем адаптер Jolt
#include "DebugDraw.hpp"
#include "Manager/ResourceManager.hpp"
#include "OpenGL/Placeholder.hpp"
#include "Renderer/Renderer.hpp" // Сначала полностью подключаем Renderer! Это решает конфликт в IRenderAPI.hpp

namespace nb::Renderer
{
    JoltDebugRenderer::JoltDebugRenderer(Renderer* renderer) : mRenderer(renderer)
    {
        JPH::DebugRenderer::Initialize();
        JPH::DebugRenderer::sInstance = this;
    }

    void JoltDebugRenderer::SetViewProj(const nb::Math::Mat4<float>& viewProj)
    {
        mViewProjMatrix = viewProj;
    }

    void JoltDebugRenderer::ResetAccumulators()
    {
        RenderLines();
        mAccumulatedLines.clear();
    }

    void JoltDebugRenderer::DrawLine(
        JPH::RVec3Arg inFrom,
        JPH::RVec3Arg inTo,
        JPH::ColorArg inColor
    )
    {
        mAccumulatedLines.push_back(
            {{inFrom.GetX(), inFrom.GetY(), inFrom.GetZ()},
             {inTo.GetX(), inTo.GetY(), inTo.GetZ()},
             {inColor.r / 255.0f, inColor.g / 255.0f, inColor.b / 255.0f}}
        );
    }

    void JoltDebugRenderer::RenderLines()
    {
        if (mAccumulatedLines.empty())
        {
            return;
        }

        for (const auto& line : mAccumulatedLines)
        {
            DebugDraw::drawLine(line.from, line.to);
        }
    }

    void JoltDebugRenderer::DrawTriangle(
        JPH::RVec3Arg inV1,
        JPH::RVec3Arg inV2,
        JPH::RVec3Arg inV3,
        JPH::ColorArg inColor,
        ECastShadow   inCastShadow
    )
    {
        DrawLine(inV1, inV2, inColor);
        DrawLine(inV2, inV3, inColor);
        DrawLine(inV3, inV1, inColor);
    }

    JPH::DebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(
        const Triangle* inTriangles,
        int             inTriangleCount
    )
    {
        std::vector<nb::Renderer::Vertex> vertices; // Явно указываем тип вершины вашего движка
        std::vector<uint32>               indices;

        vertices.reserve(inTriangleCount * 3);
        indices.reserve(inTriangleCount * 3);

        for (int i = 0; i < inTriangleCount; ++i)
        {
            const auto& tri = inTriangles[i];
            for (int v = 0; v < 3; ++v)
            {
                nb::Renderer::Vertex vertex;
                vertex.position = {
                    tri.mV[v].mPosition.x, tri.mV[v].mPosition.y, tri.mV[v].mPosition.z
                };
                vertex.normal = {tri.mV[v].mNormal.x, tri.mV[v].mNormal.y, tri.mV[v].mNormal.z};
                vertex.color  = {
                    tri.mV[v].mColor.r / 255.0f, tri.mV[v].mColor.g / 255.0f,
                    tri.mV[v].mColor.b / 255.0f
                };
                vertex.textureCoordinates = {tri.mV[v].mUV.x, tri.mV[v].mUV.y};
                vertices.push_back(vertex);
            }
            indices.push_back(i * 3 + 0);
            indices.push_back(i * 3 + 1);
            indices.push_back(i * 3 + 2);
        }

        auto mesh = std::make_shared<Mesh>(vertices, indices, "");
        return new JoltBatchImpl(mesh);
    }

    JPH::DebugRenderer::Batch JoltDebugRenderer::CreateTriangleBatch(
        const Vertex* inVertices,
        int           inVertexCount,
        const uint32* inIndices,
        int           inIndexCount
    )
    {
        std::vector<nb::Renderer::Vertex> vertices; // Явно указываем тип вершины вашего движка
        std::vector<uint32>               indices;

        vertices.reserve(inVertexCount);
        indices.reserve(inIndexCount);

        for (int i = 0; i < inVertexCount; ++i)
        {
            nb::Renderer::Vertex vertex;
            vertex.position = {
                inVertices[i].mPosition.x, inVertices[i].mPosition.y, inVertices[i].mPosition.z
            };
            vertex.normal = {
                inVertices[i].mNormal.x, inVertices[i].mNormal.y, inVertices[i].mNormal.z
            };
            vertex.color = {
                inVertices[i].mColor.r / 255.0f, inVertices[i].mColor.g / 255.0f,
                inVertices[i].mColor.b / 255.0f
            };
            vertex.textureCoordinates = {inVertices[i].mUV.x, inVertices[i].mUV.y};
            vertices.push_back(vertex);
        }

        for (int i = 0; i < inIndexCount; ++i)
        {
            indices.push_back(inIndices[i]);
        }

        auto mesh = std::make_shared<Mesh>(vertices, indices, "");
        return new JoltBatchImpl(mesh);
    }

    void JoltDebugRenderer::DrawGeometry(
        JPH::RMat44Arg     inModelMatrix,
        const JPH::AABox&  inWorldSpaceBounds,
        float              inLODScaleSq,
        JPH::ColorArg      inModelColor,
        const GeometryRef& inGeometry,
        ECullMode          inCullMode,
        ECastShadow        inCastShadow,
        EDrawMode          inDrawMode
    )
    {
        JoltBatchImpl* batch =
            static_cast<JoltBatchImpl*>(inGeometry->mLODs[0].mTriangleBatch.GetPtr());
        if (!batch || !batch->mMesh)
        {
            return;
        }

        nb::Math::Mat4<float> modelMatrix;
        for (int col = 0; col < 4; ++col)
        {
            JPH::Vec4 column    = inModelMatrix.GetColumn4(col);
            modelMatrix[col][0] = column.GetX();
            modelMatrix[col][1] = column.GetY();
            modelMatrix[col][2] = column.GetZ();
            modelMatrix[col][3] = column.GetW();
        }

        mRenderer->drawJoltGeometry(
            batch->mMesh, modelMatrix, true
        );
    }

    void JoltDebugRenderer::DrawText3D(
        JPH::RVec3Arg           inPosition,
        const std::string_view& inStr,
        JPH::ColorArg           inColor,
        float                   inHeight
    )
    {
    }
} // namespace nb::Renderer