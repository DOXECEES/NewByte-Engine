#ifndef SRC_RENDERER_SCENEGRAPH_HPP
#define SRC_RENDERER_SCENEGRAPH_HPP

#include <memory>
#include <vector> 

#include "../Math/Matrix/Mat4.hpp"

#include "Shader.hpp"
#include "Mesh.hpp"

#include "../Core.hpp"

namespace nb
{
    namespace Renderer
    {
        class SceneGraph
        {
            struct SceneNode
            {
                std::vector<std::shared_ptr<SceneNode>> childs;
                Ref<Renderer::Shader> shader;
                Ref<Renderer::Mesh> mesh;
                Math::Mat4<float> MVP;
            };

        public:

            // TODO:
            // batcher
            // instancer
            //  
            SceneGraph()
            {

            }
            
            void draw() noexcept
            {

            }

        private:
            std::shared_ptr<SceneNode> scene;
        };
    };
};

#endif

