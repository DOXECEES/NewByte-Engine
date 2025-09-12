// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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

