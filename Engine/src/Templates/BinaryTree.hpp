#ifndef SRC_TEMPLATES_BINARYTREE_HPP
#define SRC_TEMPLATES_BINARYTREE_HPP

#include <memory>
#include <functional>
#include <queue>

namespace nb
{
    namespace Templates
    {
        template <typename T>
        struct Node
        {
            explicit Node(T data)
                : data(data)
            {
            }

            T data;
            std::shared_ptr<Node> left = nullptr;
            std::shared_ptr<Node> right = nullptr;
        };

        template <typename T, typename N = Node<T>>
        class BinaryTree
        {
            public:
                
                BinaryTree() = default;

                ~BinaryTree() = default;
                
                void insert(const T& val)
                {
                    if(!this->root)
                    {
                        this->root = std::make_shared<N>(val);
                    }
                    else
                    {
                        insertFunction(this->root, val);
                    }
                    
                }

                void remove()
                {
                    
                }

                N* find(const T& val)
                {
                    
                }

                // travel
                void preOrder(const std::function<void(T)>& func)
                {
                    preOrderFunction(this->root, func);
                }

                void inOrder(const std::function<void(T)>& func)
                {
                    inOrderFunction(this->root, func);
                }

                void postOrder(const std::function<void(T)>& func)
                {
                    postOrderFunction(this->root, func);
                }

                void levelOrder(const std::function<void(T)>& func)
                {
                    levelOrderFunction(this->root, func);
                }
                //

                // На уровень первой вставляется левая нода
                // значит глубину можно считать по самой левой ноде  
                inline constexpr uint8_t depth() const noexcept 
                {
                    std::shared_ptr<N> cur = root;
                    uint8_t index = 0;
                    while(cur->left)
                    {
                        cur = cur->left;
                        index++;
                    }

                    return index;
                }

                void build(const std::vector<T>& vec) const noexcept
                {
                    for(const auto& i : vec)
                    {
                        insert(i);
                    }
                }


            protected:

                void insertFunction(const std::shared_ptr<N>& node, const T& val) const
                {
                    std::shared_ptr<N> temp;
                    std::queue<std::shared_ptr<N>> q;
                    
                    q.push(node);

                    while(!q.empty()) 
                    {
                        temp = q.front();
                        q.pop();

                        if(temp->left == nullptr)
                        {
                            temp->left = std::make_shared<N>(val);
                            break;
                        }
                        else
                        {
                            q.push(node->left);
                        }

                        if(temp->right == nullptr)
                        {
                            temp->right = std::make_shared<N>(val);
                            break;
                        }
                        else
                        {
                            q.push(temp->right);
                        }
                    }
                }

                void preOrderFunction(const std::shared_ptr<N>& node, const std::function<void(T)>& func) const
                {
                    if(node)
                    {
                        func(node->data);
                        preOrderFunction(node->left, func);
                        preOrderFunction(node->right, func);
                    }
                }

                void inOrderFunction(const std::shared_ptr<N>& node, const std::function<void(T)>& func) const
                {
                    if(node)
                    {
                        inOrderFunction(node->left, func);
                        func(node->data);
                        inOrderFunction(node->right, func);
                    }
                }

                void postOrderFunction(const std::shared_ptr<N>& node, const std::function<void(T)>& func) const
                {
                    if(node)
                    {
                        postOrderFunction(node->left, func);
                        postOrderFunction(node->right, func);
                        func(node->data);
                    }
                }

                void levelOrderFunction(const std::shared_ptr<N>& node, const std::function<void(T)>& func) const
                {
                    std::shared_ptr<N> cur = nullptr;
                    std::queue<std::shared_ptr<N>> q;
                    q.push(node);

                    while(!q.empty())
                    {
                        cur = q.front();
                        func(cur->data);
                        q.pop();

                        if(cur->left)
                            q.push(cur->left);
                        if(cur->right)
                            q.push(cur->right);
                    }
                }


                std::shared_ptr<N> root = nullptr;
        };
    };
};

#endif