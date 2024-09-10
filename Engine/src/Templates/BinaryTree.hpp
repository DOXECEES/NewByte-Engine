#ifndef SRC_TEMPLATES_BINARYTREE_HPP
#define SRC_TEMPLATES_BINARYTREE_HPP

#include <memory>

namespace nb
{
    namespace Templates
    {
        template <typename T>
        class BinaryTree
        {
            public:

                struct Node
                {
                    explicit Node(T data)
                        :data(data)
                    {

                    }
                    
                    T data = {};
                    std::shared_ptr<Node> left = nullptr;
                    std::shared_ptr<Node> right = nullptr;
                };
                
                BinaryTree() = default;

                ~BinaryTree() = default;
                
                void insert(const T& val)
                {
                    if(!root)
                    {
                        root = std::make_shared<Node>(val);
                    }
                    else
                    {
                        insertFunction(root, val);
                    }
                    
                }

                void remove()
                {
                    
                }

                Node* find(const T& val)
                {
                    
                }

                // travel
                void preOrder(const std::function<void(T)>& func)
                {
                    if(root)
                    {
                        preOrderFunction(root, func);
                    }
                }

                void inOrder(const std::function<void(T)>& func)
                {
                    if(root)
                    {
                        inOrderFunction(root, func);
                    }
                }

                void postOrder(const std::function<void(T)>& func)
                {
                    if(root)
                    {
                        postOrderFunction(root, func);
                    }
                }

                void levelOrder()
                {

                }
                //

                inline constexpr uint8_t depth() const noexcept 
                {
                    
                }


            protected:

                void insertFunction(const std::shared_ptr<Node>& node, const T& val) const
                {
                    std::shared_ptr<Node> temp;
                    std::queue<std::shared_ptr<Node>> q;
                    
                    q.push(node);

                    while(!q.empty()) 
                    {
                        temp = q.front();
                        q.pop();

                        if(temp->left == nullptr)
                        {
                            temp->left = std::make_shared<Node>(val);
                            break;
                        }
                        else
                        {
                            q.push(node->left);
                        }

                        if(temp->right == nullptr)
                        {
                            temp->right = std::make_shared<Node>(val);
                            break;
                        }
                        else
                        {
                            q.push(temp->right);
                        }
                    }
                }

                void preOrderFunction(const std::shared_ptr<Node>& node, const std::function<void(T)>& func) const
                {
                    if(node)
                    {
                        func(node->data);
                        preOrderFunction(node->left, func);
                        preOrderFunction(node->right, func);
                    }
                }

                void inOrderFunction(const std::shared_ptr<Node>& node, const std::function<void(T)>& func) const
                {
                    if(node)
                    {
                        inOrderFunction(node->left, func);
                        func(node->data);
                        inOrderFunction(node->right, func);
                    }
                }

                void postOrderFunction(const std::shared_ptr<Node>& node, const std::function<void(T)>& func) const
                {
                    if(node)
                    {
                        postOrderFunction(node->left, func);
                        postOrderFunction(node->right, func);
                        func(node->data);
                    }
                }

                std::shared_ptr<Node> root = nullptr;
        };
    };
};

#endif