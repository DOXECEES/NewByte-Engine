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
            Vertex() = default;

            Vertex(const Math::Vector3<float> &pos, const Math::Vector3<float> &norm = 0)
                : position(pos), normal(norm)
            {
            }
            Math::Vector3<float> position;
            Math::Vector3<float> normal;

            bool operator==(const Vertex &other) const
            {
                return position == other.position &&
                       normal == other.normal;
            }
            

            // Math::Vector3<float> color;
            // Math::Vector2<float> texCoords;
        };

    };
};

#endif
