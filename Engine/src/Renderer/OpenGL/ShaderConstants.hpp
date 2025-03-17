#ifndef SRC_RENDERER_SHADER_CONSTANTS_HPP
#define SRC_RENDERER_SHADER_CONSTANTS_HPP

#include <string_view>

namespace nb
{
    namespace Renderer
    {

        // Все названия uniform должны быть записаны с постфиксом UNIFORM_NAME
        // 
        namespace ShaderConstants
        {

            // Читать с конфигов
            inline static constexpr std::string_view    MODEL_MATRIX_UNIFORM_NAME              = "_modelMatrix_";
            inline static constexpr std::string_view    COUNT_OF_POINTLIGHT_UNIFORM_NAME       = "_COUNT_OF_POINTLIGHT_";
            inline static constexpr std::string_view    COUNT_OF_SPOTLIGHT_UNIFORM_NAME        = "_COUNT_OF_SPOTLIGHT_";
            inline static constexpr std::string_view    COUNT_OF_DIRECTIONLIGHT_UNIFORM_NAME   = "_COUNT_OF_DIRECTIONLIGHT_";
            inline static constexpr int                 MAX_COUNT_OF_POINTLIGHT                = 32;
            inline static constexpr int                 MAX_COUNT_OF_SPOTLIGHT                 = 8;
            inline static constexpr int                 MAX_COUNT_OF_DIRECTIONLIGHT            = 1;
        };
    };
};

#endif
