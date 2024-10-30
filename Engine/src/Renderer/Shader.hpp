#ifndef SRC_RENDERER_SHADER_HPP
#define SRC_RENDERER_SHADER_HPP

#include "../Resources/IResource.hpp"

namespace nb
{
    namespace Renderer
    {
        class Shader : public Resource::IResource
        {
        public:
            virtual ~Shader() noexcept = default;

            virtual void use() noexcept = 0;



        private:
        };
    };
};

#endif
