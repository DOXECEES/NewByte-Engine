#ifndef SRC_RENDERER_RENDERERSTRUCTURES_HPP
#define SRC_RENDERER_RENDERERSTRUCTURES_HPP

#include "../Math/Vector2.hpp"
#include "../Math/Vector3.hpp"


#include <vector>

namespace nb
{
    namespace Renderer
    {
        

        struct Vertex
        {
            Vertex(Math::Vector3<float> pos)
                : position(pos)
            {}
            Math::Vector3<float> position;
            // Math::Vector3<float> normal;
            // Math::Vector3<float> color;
            // Math::Vector2<float> texCoords;
        };
        
    };
};

#endif
