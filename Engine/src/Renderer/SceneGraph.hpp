#ifndef SRC_RENDERER_SCENEGRAPH_HPP
#define SRC_RENDERER_SCENEGRAPH_HPP

#include <memory>
#include <vector> 

namespace nb
{
    namespace Renderer
    {
        class SceneGraph
        {

            struct SceneNode
            {
                std::vector<std::shared_ptr<SceneNode>> childs;
                // MESH
                // SHADER
                // MVP
            };

        public:

        private:
            std::shared_ptr<SceneNode> scene;
        };
    };
};

#endif

