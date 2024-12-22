#include "Shader.hpp"

namespace nb
{
    namespace Renderer
    {
        Shader::Shader() noexcept
            : id(globalId++)
        {
        }

        int Shader::globalId = 0;
    };
};

