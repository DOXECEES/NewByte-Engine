#pragma once

#include <Reflection/Reflection.hpp>

#include "Renderer/Animator.hpp"
#include "Renderer/Animation.hpp"

#include <memory>

namespace nb
{
    struct AnimatorComponent
    {
        std::unique_ptr<Renderer::Animator> animator;
        std::shared_ptr<Renderer::Animation> currentAnimation;
        bool isPlaying = true;
        float speed = 1.0f;

        AnimatorComponent()
        {
            nb::Error::ErrorManager::instance().report(Error::Type::WARNING, "Create");
        }

        AnimatorComponent(std::shared_ptr<Renderer::Animation> animation)
            : currentAnimation(animation)
        {
            if (animation)
            {
                animator = std::make_unique<Renderer::Animator>(animation.get());
            }
        }
    };
}

NB_REFLECT_PTR(
    std::shared_ptr<nb::Renderer::Animation>,
    "std::shared_ptr<nb::Renderer::Animation>"
)

NB_REFLECT_RESOURCE_PTR(
    std::shared_ptr<nb::Renderer::Animation>,
    "AnimationPtr",
    [](std::shared_ptr<nb::Renderer::Animation>* field,
       const std::string& path)
    {
        *field =
            nb::ResMan::ResourceManager::getInstance()->getResource<nb::Renderer::Animation>(path);
    }
)


NB_REFLECT_STRUCT(nb::AnimatorComponent,
    NB_FIELD(nb::AnimatorComponent, currentAnimation),
    NB_FIELD(nb::AnimatorComponent, isPlaying),
    NB_FIELD(nb::AnimatorComponent, speed)
)
