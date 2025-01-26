#include "SceneGraph.hpp"
#include "../Math/Matrix/Transformation.hpp"

#include "../Debug.hpp"

namespace nb
{
    namespace Renderer
    {
        const std::vector<std::shared_ptr<BaseNode>> &BaseNode::getChildrens() const noexcept
        {
            return children;
        }

        /// Nodes
        std::shared_ptr<BaseNode> BaseNode::getChildren(const size_t index) const noexcept
        {
            return (this->children.size() > index) ? this->children[index] : nullptr;
        }

        void BaseNode::addChildren(std::shared_ptr<BaseNode> node) noexcept
        {
            node->parent = shared_from_this();
            this->children.push_back(node);
            SceneGraph::treeMap[node->getName()] = node;
            dirtyFlag = true;
        }

        void BaseNode::setTranslate(const Math::Vector3<float> &translate) noexcept
        {
            transform.translate = translate;
            dirtyFlag = true;
        }

        void BaseNode::setRotationX(const float val) noexcept
        {
            transform.rotateX = Math::toRadians(val);
            dirtyFlag = true;
        }

        void BaseNode::setRotationY(const float val) noexcept
        {
            transform.rotateY = Math::toRadians(val);
            dirtyFlag = true;
        }

        void BaseNode::setRotationZ(const float val) noexcept
        {
            transform.rotateZ = Math::toRadians(val);
            dirtyFlag = true;
        }

        void BaseNode::setScale(const Math::Vector3<float>& scale) noexcept
        {
            transform.scale = scale;
            dirtyFlag = true;
        }

        void BaseNode::setTransform(const Transform &newTransform) noexcept
        {
            this->transform = newTransform;
        }

        Math::Mat4<float> BaseNode::getLocalTransform() const noexcept
        {
            return localTransformation;
        }

        Math::Mat4<float> BaseNode::getWorldTransform() const noexcept
        {
            return worldTransformation;
        }

        void BaseNode::setName(std::string_view str) noexcept
        {
            name = std::move(str.data());
        }

        std::string BaseNode::getName() const noexcept
        {
            return name;
        }

        std::shared_ptr<BaseNode> BaseNode::getParent() const noexcept
        {
            return parent;
        }

        const Transform &BaseNode::getTransform() const noexcept
        {
            return transform;
        }

        Math::Mat4<float> BaseNode::getLocalTransform() noexcept
        {
            auto scale = Math::scale(localTransformation, transform.scale);
            auto rotX = Math::rotate(localTransformation, {1.0f, 0.0f, 0.0f}, transform.rotateX);
            auto rotY = Math::rotate(localTransformation, {0.0f, 1.0f, 0.0f}, transform.rotateY);
            auto rotZ = Math::rotate(localTransformation, {0.0f, 0.0f, 1.0f}, transform.rotateZ);
            auto transl = Math::translate(localTransformation, transform.translate);

            auto rot = rotX * rotY * rotZ;

            return scale * rot * transl;
        }

        void BaseNode::updateWorldTransform() noexcept
        {
            if (dirtyFlag == true)
            {
                Debug::debug("dirty");
                updateWorldTransformForce();
                return;
            }
            
            for (auto &i : children)
            {
                i->updateWorldTransform();
            }
        }

        void BaseNode::updateWorldTransformForce() noexcept
        {
            if(parent == nullptr)
            {
                this->worldTransformation = getLocalTransform();
                dirtyFlag = false;
            }
            else
            {
                this->worldTransformation = getLocalTransform() * parent->getWorldTransform();
                dirtyFlag = false;
            }

            for (auto &i : children)
            {
                i->updateWorldTransformForce();
            }

        }

        ///

        std::shared_ptr<SceneGraph> SceneGraph::getInstance() noexcept
        {
            static std::shared_ptr<SceneGraph> instance = std::make_shared<SceneGraph>();
            return instance;
        }

        std::shared_ptr<BaseNode> SceneGraph::getScene() const noexcept
        {
            return scene;
        }

        std::shared_ptr<BaseNode> SceneGraph::find(std::string_view str)
        {
            return treeMap[str.data()];
        }

        SceneGraph::SceneGraph() noexcept
        {
            scene = std::make_shared<BaseNode>("root", Transform());
            treeMap["root"] = scene;
        }


        std::shared_ptr<BaseNode> SceneGraph::scene = std::make_shared<BaseNode>("root", Renderer::Transform());
        std::unordered_map<std::string, std::shared_ptr<BaseNode>> SceneGraph::treeMap;

    };
};
