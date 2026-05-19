#include "DebugDraw.hpp"
#include "Camera.hpp"
#include "Core/EngineSettings.hpp"
#include "IRenderAPI.hpp"
#include "Manager/ResourceManager.hpp"

namespace nb::Renderer
{
    namespace
    {
        const char* const LINE_SHADER_PATH  = "line.shader";
        const char* const VIEW_UNIFORM      = "u_View";
        const char* const PROJ_UNIFORM      = "u_Proj";
        const char* const THICKNESS_UNIFORM = "u_Thickness";
        const char* const VIEWPORT_UNIFORM    = "u_ViewportSize";

        constexpr float    POSITIVE_SIDE     = 1.0f;
        constexpr float    NEGATIVE_SIDE     = -1.0f;
        constexpr uint32_t VERTICES_PER_LINE = 4;
        constexpr uint32_t INDICES_PER_LINE  = 6;
    } // namespace

    void DebugDraw::drawLine(
        const Point&                p1,
        const Point&                p2,
        const Math::Vector3<float>& color
    ) noexcept
    {
        if (batchVertices.empty())
        {
            batchVertices.reserve(INITIAL_BATCH_RESERVE);
            batchIndices.reserve(INITIAL_BATCH_RESERVE * 1.5);
        }

        const uint32_t startIndex = static_cast<uint32_t>(batchVertices.size());

        batchVertices.push_back({p1, p2, color, POSITIVE_SIDE});
        batchVertices.push_back({p1, p2, color, NEGATIVE_SIDE});
        batchVertices.push_back({p2, p1, color, POSITIVE_SIDE});
        batchVertices.push_back({p2, p1, color, NEGATIVE_SIDE});

        batchIndices.push_back(startIndex + 0);
        batchIndices.push_back(startIndex + 1);
        batchIndices.push_back(startIndex + 2);
        batchIndices.push_back(startIndex + 2);
        batchIndices.push_back(startIndex + 1);
        batchIndices.push_back(startIndex + 3);
    }

    void DebugDraw::drawCircle(
        const Math::Vector3<float>& center,
        float                       radius,
        int                         segments,
        const Math::Vector3<float>& normal
    ) noexcept
    {
        using Vec3 = Math::Vector3<float>;

        Vec3 up    = (std::abs(normal.y) > 0.999f) ? Vec3(1, 0, 0) : Vec3(0, 1, 0);
        Vec3 right = Math::normalize(Math::cross(up, normal));
        up         = Math::normalize(Math::cross(normal, right));

        Vec3 prevPoint = center + right * radius;

        for (int i = 1; i <= segments; ++i)
        {
            float angle = (static_cast<float>(i) / segments) * 2.0f * Math::Constants::PI;

            Vec3 currentPoint =
                center + right * (cosf(angle) * radius) + up * (sinf(angle) * radius);

            DebugDraw::drawLine(prevPoint, currentPoint);
            prevPoint = currentPoint;
        }
    }

    void DebugDraw::drawBatch(
        nbstl::NonOwningPtr<IRenderAPI> api,
        nbstl::NonOwningPtr<Camera>     camera
    ) noexcept
    {
        if (batchVertices.empty())
        {
            return;
        }

        if (!api)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "RenderAPI pointer is null")
                .with("context", "DebugDraw::drawBatch");
            return;
        }

        if (!camera)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "Camera pointer is null")
                .with("context", "DebugDraw::drawBatch");
            return;
        }

        auto* resourceManager = ResMan::ResourceManager::getInstance();
        if (!resourceManager)
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::FATAL, "ResourceManager instance is null"
            );
            return;
        }

        auto shader = resourceManager->getResource<Shader>(LINE_SHADER_PATH);
        if (!shader)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "Failed to load line shader")
                .with("path", LINE_SHADER_PATH);
            return;
        }

        shader->use();

        const float screenWidth  = static_cast<float>(nb::Core::EngineSettings::getWidth());
        const float screenHeight = static_cast<float>(nb::Core::EngineSettings::getHeight());

        if (screenHeight <= 0.0f)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::WARNING, "Engine screen height is zero or negative")
                .with("height", screenHeight);
            return;
        }

        const float aspect = screenWidth / screenHeight;

        shader->setUniformMat4(VIEW_UNIFORM, camera->getLookAt());
        shader->setUniformMat4(PROJ_UNIFORM, camera->getProjection());
        shader->setUniformFloat(THICKNESS_UNIFORM, thickness);
        shader->setUniformVec2(VIEWPORT_UNIFORM, {screenWidth, screenHeight});

        Renderer::VertexLayout layout;
        layout.stride     = sizeof(DebugVertex);
        layout.attributes = {
            {0, 3, GL_FLOAT, offsetof(DebugVertex, currentPos)},
            {1, 3, GL_FLOAT, offsetof(DebugVertex, otherPos)},
            {2, 3, GL_FLOAT, offsetof(DebugVertex, color)},
            {3, 1, GL_FLOAT, offsetof(DebugVertex, side)}
        };

        api->drawIndexedBuffer(
            nbstl::Span<DebugVertex>{batchVertices}.toBytes(),
            nbstl::Span<uint32_t>{batchIndices},
            PrimitiveType::TRIANGLE,
            layout
        );

        batchVertices.clear();
        batchIndices.clear();
    }

    void DebugDraw::setThickness(float newThickness) noexcept
    {
        if (newThickness <= 0.0f)
        {
            thickness = DEFAULT_THICKNESS;
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::WARNING, "Thickness less or equal to zero")
                .with("thickness", newThickness);
            return;
        }

        thickness = newThickness;
    }
} // namespace nb::Renderer