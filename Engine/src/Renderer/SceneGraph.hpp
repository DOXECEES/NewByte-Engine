#ifndef SRC_RENDERER_SCENEGRAPH_HPP
#define SRC_RENDERER_SCENEGRAPH_HPP

#include <memory>
#include <vector>
#include <stack>
#include <unordered_map>
#include <functional>

#include "../Math/Matrix/Matrix.hpp"
#include "../Math/Math.hpp" 

#include "Shader.hpp"
#include "Mesh.hpp"
#include "Light.hpp"

#include "../Core.hpp"

#include "ECS/ecs.hpp"

namespace nb
{
    namespace Renderer
    {
        class SceneGraph;
        class TransformProxy;

        class BaseVisitor
        {
        public:
            BaseVisitor() = default;
            virtual void visitObjectNode() noexcept = 0;
        };

        class ObjectVisitor : public BaseVisitor
        {
        public:
            ObjectVisitor() = default;
            virtual void visitObjectNode() noexcept override;
        };

        struct Transform
        {
            Transform() noexcept = default;
            // Transform(const Math::Vector3<float> scale, const Math::Vector3<float> translate, float x, float y, float z)
            // : scale(scale), translate(translate), rotateX(x), rotateY(y),rotateZ(z) {}
            ~Transform() = default;

            Math::Vector3<float> scale = Math::Vector3<float>({1.0f,1.0f,1.0});
            float rotateX = 0.0f;
            float rotateY = 0.0f;
            float rotateZ = 0.0f;
            Math::Vector3<float> translate = Math::Vector3<float>({0.0f,0.0f,0.0f});
        };

        class BaseNode : public std::enable_shared_from_this<BaseNode>
        {

        public:
            BaseNode() = delete;
            BaseNode(std::string_view nodeName, const Transform& nodeTransform)
                : name(nodeName.data())
                , transform(nodeTransform) 
                , ecsId()
            {
            }
            virtual ~BaseNode() = default;

            void moveAt(const Math::Vector3<float> &direction) noexcept;

            const std::vector<std::shared_ptr<BaseNode>> &getChildrens() const noexcept;

            std::shared_ptr<BaseNode> getChildren(const size_t index) const noexcept;
            void addChildren(std::shared_ptr<BaseNode> node) noexcept;

            void addTranslate(const Math::Vector3<float>& translate) noexcept;
            void setTranslate(const Math::Vector3<float>& translate) noexcept;
            void setRotationX(const float val) noexcept;
            void setRotationY(const float val) noexcept;
            void setRotationZ(const float val) noexcept;
            void setScale(const Math::Vector3<float>& scale) noexcept;


            void setTransform(const Transform &newTransform) noexcept;
            void setTransformX(float value) noexcept;
            void setTransformY(float value) noexcept;
            void setTransformZ(float value) noexcept;

            Math::Mat4<float> getLocalTransform() const noexcept;
            Math::Mat4<float> getWorldTransform() const noexcept;

            const Math::Vector3<float> &getPosition() const noexcept;

            void setName(std::string_view str) noexcept;
            std::string getName() const noexcept;

            std::shared_ptr<BaseNode> getParent() const noexcept;

            const Transform& getTransform() const noexcept;

            TransformProxy getTransformView() noexcept;

            Math::Mat4<float> getLocalTransform() noexcept;
            void updateWorldTransform() noexcept;
            void updateWorldTransformForce() noexcept;

            virtual void onDirtyChange() noexcept {};

            void markDirty() noexcept;

        protected:

            Ecs::ECSRegistry ecs;
            Ecs::EntityID ecsId;

            Transform                               transform;
            
            Math::Mat4<float>                       localTransformation = Math::Mat4<float>::identity();
            Math::Mat4<float>                       worldTransformation = Math::Mat4<float>::identity();

            std::shared_ptr<BaseNode>               parent;
            std::vector<std::shared_ptr<BaseNode>>  children;
            std::string                             name;

            bool dirtyFlag = true;
        };

        class ObjectNode : public BaseNode
        {
        public:
            ObjectNode() = delete;
            ObjectNode(
                std::string_view nodeName,
                const Transform& nodeTransform,
                const std::shared_ptr<Mesh> &nodeMesh,
                const std::shared_ptr<Shader>& nodeShader
            )
                : BaseNode(nodeName, nodeTransform), mesh(nodeMesh)
            {
                mesh->uniforms.shader = nodeShader;
            }

        //private:
            std::shared_ptr<Mesh>   mesh;
            Ref<Material>           material;
            
        };

        class LightNode : public BaseNode
        {
            public:
            LightNode() = delete;
            LightNode(std::string_view nodeName, const Transform& nodeTransform, const std::shared_ptr<Light> &nodeLight)
                : BaseNode(nodeName, nodeTransform), light(nodeLight)
            {
            }
            
            void onDirtyChange() noexcept override
            {
                light->setPosition(transform.translate);
            }

            //private:
                std::shared_ptr<Light> light;
        };

        class SceneGraph
        {
            friend class BaseNode;

        public:
            static std::shared_ptr<SceneGraph> getInstance() noexcept;
            std::shared_ptr<BaseNode> getScene() const noexcept;

            std::shared_ptr<BaseNode> find(std::string_view str);

            void traverse(const std::function<void(std::shared_ptr<BaseNode>&)>& func) noexcept;

            SceneGraph() noexcept;

        private:

            static std::shared_ptr<BaseNode>                                    scene;
            static std::unordered_map<std::string, std::shared_ptr<BaseNode>>   treeMap;
        };
    };
};

#endif
