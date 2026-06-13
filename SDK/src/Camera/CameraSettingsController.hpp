#ifndef SDK_CAMERA_CAMERASETTINGSCONTROLLER_HPP
#define SDK_CAMERA_CAMERASETTINGSCONTROLLER_HPP

#include <NonOwningPtr.hpp>

#include "CameraSettings.hpp"

namespace nb::Core
{
    class Engine;
};

namespace sdk
{
    class CameraSettingsController
    {

    public:
       

        CameraSettingsController() noexcept = default;

        void update(const CameraSettings& settings) const noexcept;
        
        void init(nbstl::NonOwningPtr<nb::Core::Engine> engine) noexcept;


    private:
        nbstl::NonOwningPtr<nb::Core::Engine> engine = nullptr;
    };
};

#endif