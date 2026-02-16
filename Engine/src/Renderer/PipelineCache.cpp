#include "PipelineCache.hpp"

#include "IRenderAPI.hpp"

namespace nb::Renderer
{
    PipelineHandle PipelineCache::getOrCreate(const Pipeline& desc) noexcept
    {

        uint32 hash = nbstl::xxHash32(&desc, sizeof(Pipeline));

        if (auto it = hashToHandle.find(hash); it != hashToHandle.end())
        {
            return it->second;
        }

        PipelineHandle newHandle = createNewPipeline(desc);

        hashToHandle[hash] = newHandle;
        handleToDesc[newHandle] = desc;

        return newHandle;
    }

    const Pipeline& PipelineCache::getDesc(PipelineHandle handle) const noexcept
    {
        return handleToDesc.at(handle);
    }

    PipelineHandle PipelineCache::createNewPipeline(const Pipeline& desc) noexcept
    {
        static PipelineHandle counter = 1;
        return counter++;
    }
};

