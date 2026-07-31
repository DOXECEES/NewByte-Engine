#ifndef SRC_RENDERER_POSTEFFECTS_DEPTHOFFIELD_HPP
#define SRC_RENDERER_POSTEFFECTS_DEPTHOFFIELD_HPP

#include "Renderer/PostProcess.hpp"

#include <Reflection/Reflection.hpp>

namespace nb::Renderer::PostEffects
{
    struct DofGpuData
    {
        float nearPlane = 0.1f;
        float farPlane = 100.0f;
        float focusDistance = 15.0f;
        float focusRange = 100.0f;
    };


    struct DepthOfField : public PostProcessEffectConfig 
    {
        static constexpr PostProcessEffect EffectType = PostProcessEffect::Dof;
        using EffectGpuData = DofGpuData;


        PostProcessEffect getPostEffectType() const noexcept override { return EffectType; }

        DofGpuData data;  
    };
};

NB_REFLECT_STRUCT(nb::Renderer::PostEffects::DofGpuData, 
    NB_FIELD(nb::Renderer::PostEffects::DofGpuData, nearPlane),
    NB_FIELD(nb::Renderer::PostEffects::DofGpuData, farPlane),
    NB_FIELD(nb::Renderer::PostEffects::DofGpuData, focusDistance),
    NB_FIELD(nb::Renderer::PostEffects::DofGpuData, focusRange)
);

#endif