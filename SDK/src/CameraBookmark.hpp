#ifndef SDK_CAMERABOOKMARK_HPP
#define SDK_CAMERABOOKMARK_HPP

#include <NbCore.hpp>

#include <Math/Vector3.hpp>
#include <Renderer/Camera.hpp>

#include <NonOwningPtr.hpp>

#include <array>
#include <string>

namespace sdk
{
    struct CameraBookmark
    {
        nb::Math::Vector3<float> position   = {0.0f, 0.0f, 0.0f};
        nb::Math::Vector3<float> direction = {0.0f, 0.0f, 1.0f};
        bool isValid = false;
    };

    class CameraBookmarkManager
    {
    public:
        constexpr static size_t MAX_BOOKMARKS = 10; // buttons from 0 to 9

        CameraBookmarkManager() = default;
        ~CameraBookmarkManager() = default;

        NB_NON_COPYMOVABLE(CameraBookmarkManager);

        bool hasBookmark(size_t index) const noexcept; 

        void record(size_t index, nbstl::NonOwningPtr<nb::Renderer::Camera> camera) noexcept;
        void apply(size_t index, nbstl::NonOwningPtr<nb::Renderer::Camera> camera) noexcept;
        void applyInterpolated(size_t index, nbstl::NonOwningPtr<nb::Renderer::Camera> camera, float t) noexcept;
        auto getBookmarks() const noexcept -> const std::array<CameraBookmark, MAX_BOOKMARKS>&;

    private:
        std::array<CameraBookmark, MAX_BOOKMARKS> bookmarks = {};
        
    };

    
};


#endif