#include "PostProcess.hpp"

namespace nb::Renderer
{
    PostProcess::PostProcess(const nbstl::NonOwningPtr<IRenderAPI> renderApi) noexcept
        : api(renderApi)
    {
    }

    void PostProcess::addEffect(PostProcessEffect effect)
    {
        size_t index = static_cast<size_t>(effect);
        activeEffects.set(index);
    }

    void PostProcess::removeEffect(PostProcessEffect effect)
    {   
        activeEffects.reset(static_cast<size_t>(effect));
    }
    bool PostProcess::isEffectActive(PostProcessEffect effect) const
    {
        return activeEffects.test(static_cast<size_t>(effect));
    }
} // namespace nb::Renderer