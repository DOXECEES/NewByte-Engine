// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "CameraBookmark.hpp"

#include <Error/ErrorManager.hpp>


namespace sdk
{
    bool CameraBookmarkManager::hasBookmark(size_t index) const noexcept
    {
        if(index >= MAX_BOOKMARKS)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "Invalid bookmark index")
                .with("Index", index);
            return false;
        }

        return bookmarks[index].isValid;
    }

    void CameraBookmarkManager::record(
        size_t                                    index,
        nbstl::NonOwningPtr<nb::Renderer::Camera> camera
    ) noexcept
    {
        if (index < MAX_BOOKMARKS)
        {
            bookmarks[index].position = camera->getPosition();
            bookmarks[index].direction = camera->getDirection();
            bookmarks[index].isValid = true;
        }
    }

    void CameraBookmarkManager::apply(
        size_t                                    index,
        nbstl::NonOwningPtr<nb::Renderer::Camera> camera
    ) noexcept
    {
        if(index < MAX_BOOKMARKS && bookmarks[index].isValid)
        {
            camera->moveTo(bookmarks[index].position);
            camera->setDirection(bookmarks[index].direction);
        }
    }


    [[nodiscard]] static constexpr nb::Math::Vector3<float> lerp(
                const nb::Math::Vector3<float>& start, 
                const nb::Math::Vector3<float>& end, 
                float              t
            ) noexcept
            {
                return nb::Math::Vector3<float>{
                    start.x + (end.x - start.x) * t,
                    start.y + (end.y - start.y) * t,
                    start.z + (end.z - start.z) * t
                };
            }

    void CameraBookmarkManager::applyInterpolated(
        size_t                                    index,
        nbstl::NonOwningPtr<nb::Renderer::Camera> camera,
        float                                     t
    ) noexcept
    {
        if (index < MAX_BOOKMARKS)
        {
            const auto& target = bookmarks[index];
            
            auto currentPos = camera->getPosition();
            auto currentDir = camera->getDirection();

            auto nextPos = lerp(currentPos, target.position, t);
            
            auto nextDir = lerp(currentDir, target.direction, t);
            nextDir.normalize(); 

            camera->moveTo(nextPos);
            camera->setDirection(nextDir);
        }
    }
    
    auto CameraBookmarkManager::getBookmarks() const noexcept -> const std::array<CameraBookmark, MAX_BOOKMARKS>& 
    {
        return bookmarks;
    }
} // namespace sdk
