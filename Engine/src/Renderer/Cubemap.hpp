#ifndef SRC_RENDERER_CUBEMAP_HPP
#define SRC_RENDERER_CUBEMAP_HPP

#include <cstdint>

namespace nb::Renderer
{

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