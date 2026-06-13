#ifndef SDK_ENGINESETTINGSCONTROLLER_HPP
#define SDK_ENGINESETTINGSCONTROLLER_HPP

#include <Core/Engine.hpp>

#include <NonOwningPtr.hpp>

namespace sdk
{
    class EngineSettingsController
    {
    public:
        EngineSettingsController(nbstl::NonOwningPtr<nb::Core::Engine> engine) noexcept
            :engine(engine)
        {}
        ~EngineSettingsController() = default;
    
        void setDebugRendererSettings(const nb::Renderer::DebugRendererSettings& settings) const noexcept
        {
            if(auto renderer = engine->getRenderer(); renderer != nullptr)
            {
                renderer->setDebugSettings(settings);
            }
        }

    private:
        nbstl::NonOwningPtr<nb::Core::Engine> engine;

    };
    
};

#endif
