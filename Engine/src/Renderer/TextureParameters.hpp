#ifndef SRC_RENDERER_TEXTUREPARAMETERS_HPP
#define SRC_RENDERER_TEXTUREPARAMETERS_HPP

namespace nb::Renderer
{
    enum class Format
    {
        Rgba8UNormal,
        Rgba16Float,
        Rgba32Float,
        R32Float,
    };

    enum class Filtering
    {
        Linear,
        Nearest
    };

    enum class Wrapping
    {
        Repeat,
        MirroredRepeat,
        ClampToEdge,
        ClampToBorder
    };
};


#endif