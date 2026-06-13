#include "CameraSettingsController.hpp"

#include <Core/Engine.hpp>
#include <Error/ErrorManager.hpp> 

namespace sdk
{
    void CameraSettingsController::update(const CameraSettings& settings) const noexcept
    {
        if (!engine)
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::WARNING, "Engine is not initialized in CameraSettingsController"
            );
            return;
        }
        engine->setCameraSpeed(settings.speed);
        engine->setCameraSpeedMultiplier(settings.multiplier);
    }

    void CameraSettingsController::init(nbstl::NonOwningPtr<nb::Core::Engine> engine) noexcept
    {
        this->engine = engine;
    }

}; // namespace sdk
