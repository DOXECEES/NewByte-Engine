#ifndef SRC_RENDERER_CUBEMAP_HPP
#define SRC_RENDERER_CUBEMAP_HPP

#include <cstdint>

namespace nb::Renderer
{

    struct CubemapParameters
    {
        uint32_t size = 512;

        enum class Format
        {
            Rgba8UNormal,  
            Rgba16Float, 
            Rgba32Float,
            R32Float,
        };
        Format format = Format::Rgba8UNormal;

        enum class Filtering
        {
            Linear,
            Nearest
        };
        Filtering magFilter = Filtering::Linear;
        Filtering minFilter = Filtering::Linear;

        enum class Wrapping
        {
            Repeat,
            MirroredRepeat,
            ClampToEdge,
            ClampToBorder
        };
        Wrapping wrapU = Wrapping::ClampToEdge;
        Wrapping wrapV = Wrapping::ClampToEdge;
        Wrapping wrapW = Wrapping::ClampToEdge;

        bool generateMipmaps = true;
    };



    class Cubemap
    {
    public:
        virtual ~Cubemap() noexcept = default;

        virtual uint32_t getId() const noexcept = 0;
        virtual uint64_t getHandle() const noexcept = 0; 

    private:

    };

};

#endif