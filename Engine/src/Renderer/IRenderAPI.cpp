// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "IRenderAPI.hpp"

namespace nb
{
    namespace Renderer
    {
        IRenderAPI::IRenderAPI(void* handle) noexcept
            : pipelineCache(this)
        {
        }

        void IRenderAPI::enableLightVisualization() noexcept
        {
            shouldVisualizeLight = true;
        }

        void IRenderAPI::disableLightVisualization() noexcept
        {
            shouldVisualizeLight = false;
        }

        void IRenderAPI::toggleLightVisualization() noexcept
        {
            shouldVisualizeLight = !shouldVisualizeLight;
        }

        PipelineCache& IRenderAPI::getCache() noexcept
        {
            return pipelineCache;
        }

    }
};

