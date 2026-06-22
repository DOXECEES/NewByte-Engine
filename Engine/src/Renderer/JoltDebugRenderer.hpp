#pragma once
#include "Renderer.hpp" // ← добавить сюда

#include <Jolt/Jolt.h>
#include <Jolt/Renderer/DebugRenderer.h>

#include "Math/Matrix/Matrix.hpp"
#include "Math/Vector3.hpp"
#include "Renderer/Objects/Objects.hpp"

namespace nb::Renderer
{
    class Renderer; // Forward declaration

    class JoltBatchImpl : public JPH::RefTargetVirtual
    {
    public:
        JoltBatchImpl(Ref<Mesh> mesh) : mMesh(mesh)
        {
        }

        virtual void AddRef() override
        {
            ++mRefCount;
        }
        virtual void Release() override
        {
            if (--mRefCount == 0)
            {
                delete this;
            }
        }

        Ref<Mesh> mMesh;

    private:
        std::atomic<uint32_t> mRefCount{0};
    };

    class JoltDebugRenderer : public JPH::DebugRenderer
    {
    public:
        JoltDebugRenderer(Renderer* renderer);
        virtual ~JoltDebugRenderer() override = default;

        virtual void DrawLine(
            JPH::RVec3Arg inFrom,
            JPH::RVec3Arg inTo,
            JPH::ColorArg inColor
        ) override;
        virtual void DrawTriangle(
            JPH::RVec3Arg inV1,
            JPH::RVec3Arg inV2,
            JPH::RVec3Arg inV3,
            JPH::ColorArg inColor,
            ECastShadow   inCastShadow
        ) override;

        virtual Batch CreateTriangleBatch(
            const Triangle* inTriangles,
            int             inTriangleCount
        ) override;
        virtual Batch CreateTriangleBatch(
            const Vertex* inVertices,
            int           inVertexCount,
            const uint32* inIndices,
            int           inIndexCount
        ) override;

        // Переопределяем только валидный виртуальный метод DrawGeometry с 8 аргументами
        virtual void DrawGeometry(
            JPH::RMat44Arg     inModelMatrix,
            const JPH::AABox&  inWorldSpaceBounds,
            float              inLODScaleSq,
            JPH::ColorArg      inModelColor,
            const GeometryRef& inGeometry,
            ECullMode          inCullMode,
            ECastShadow        inCastShadow,
            EDrawMode          inDrawMode
        ) override;

        virtual void DrawText3D(
            JPH::RVec3Arg           inPosition,
            const std::string_view& inStr,
            JPH::ColorArg           inColor,
            float                   inHeight
        ) override;

        void SetViewProj(const nb::Math::Mat4<float>& viewProj);
        void ResetAccumulators();
        void RenderLines();


    private:
        Renderer*             mRenderer;
        nb::Math::Mat4<float> mViewProjMatrix;

        struct LineInfo
        {
            nb::Math::Vector3<float> from;
            nb::Math::Vector3<float> to;
            nb::Math::Vector3<float> color;
        };
        std::vector<LineInfo> mAccumulatedLines;
    };
} // namespace nb::Renderer