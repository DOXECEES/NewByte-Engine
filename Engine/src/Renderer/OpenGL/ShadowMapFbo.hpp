#ifndef SRC_RENDERER_OPENGL_SHADOWMAPFBO_HPP
#define SRC_RENDERER_OPENGL_SHADOWMAPFBO_HPP

#include "FBO.hpp"

namespace nb
{
    namespace OpenGl
    {

        class ShadowMapFbo : public FBO
        {
        public:
            ShadowMapFbo();
            ~ShadowMapFbo();
        };

    };
};

#endif