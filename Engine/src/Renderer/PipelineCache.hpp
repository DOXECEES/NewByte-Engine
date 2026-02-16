#ifndef SRC_RENDERER_PIPELINECACHE_HPP
#define SRC_RENDERER_PIPELINECACHE_HPP

#include <NbCore.hpp>
#include <Types.hpp>
#include <Hash.hpp>
#include <map>

namespace nb::Renderer
{
    class IRenderAPI;
    struct Pipeline;

    using PipelineHandle = uint32;

    class PipelineCache
    {
    public:
        NB_NON_COPYMOVABLE(PipelineCache);

        PipelineCache(IRenderAPI* api) noexcept : api(api) {}
        ~PipelineCache() noexcept = default;
       
        PipelineHandle getOrCreate(const Pipeline& desc) noexcept;
        
        const Pipeline& getDesc(PipelineHandle handle) const noexcept;
        

    private:
        PipelineHandle createNewPipeline(const Pipeline& desc) noexcept;
        

    private:
        IRenderAPI* api;

        std::map<uint32, PipelineHandle> hashToHandle;

        std::map<PipelineHandle, Pipeline> handleToDesc;
    };
};

#endif