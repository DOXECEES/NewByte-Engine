#ifndef SRC_RENDERER_TEXTURE_HPP
#define SRC_RENDERER_TEXTURE_HPP

#include "../Resources/IResource.hpp"


namespace nb
{
    namespace Renderer
    {
        class Texture : public Resource::IResource
        {
        public:
            virtual ~Texture() noexcept = default;

            virtual uint32_t getId() const noexcept = 0;

            virtual int getWidth() const noexcept = 0;
            virtual int getHeight() const noexcept = 0;

        private:
        };
    };
};

#endif

