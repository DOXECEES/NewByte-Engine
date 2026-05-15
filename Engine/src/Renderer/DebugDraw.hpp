#ifndef SRC_RENDERER_DEBUGDRAW_HPP
#define SRC_RENDERER_DEBUGDRAW_HPP

#include "Math/Vector3.hpp"
#include <NonOwningPtr.hpp>
#include <vector>

namespace nb::Renderer
{
    class IRenderAPI;
    class Camera;

    struct DebugVertex
    {
        Math::Vector3<float> currentPos;
        Math::Vector3<float> otherPos;
        Math::Vector3<float> color;
        float                side;
    };

    using Point = Math::Vector3<float>;

    class DebugDraw final
    {
    public:
        static inline const Math::Vector3<float> DEFAULT_COLOR{1.0f, 0.5f, 0.0f};
        static constexpr float                   DEFAULT_THICKNESS     = 2.0f;
        static constexpr size_t                  INITIAL_BATCH_RESERVE = 1024;

        DebugDraw() = delete;

        static void drawLine(
            const Point&                p1,
            const Point&                p2,
            const Math::Vector3<float>& color     = DEFAULT_COLOR
        ) noexcept;


        static void drawCircle(
            const Math::Vector3<float>& center,
            float                       radius,
            int                         segments,
            const Math::Vector3<float>& normal
        ) noexcept;
        


        static void drawBatch(
            nbstl::NonOwningPtr<IRenderAPI> api,
            nbstl::NonOwningPtr<Camera>     camera
        ) noexcept;

        static void setThickness(float newThickness) noexcept;


    private:
        static inline std::vector<DebugVertex> batchVertices;
        static inline std::vector<uint32_t>    batchIndices;
        static inline float                    thickness = DEFAULT_THICKNESS;
    };
} // namespace nb::Renderer

#endif // SRC_RENDERER_DEBUGDRAW_HPP