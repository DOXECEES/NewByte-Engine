#ifndef SRC_TEMPLATES_BINARYTREE_HPP
#define SRC_TEMPLATES_BINARYTREE_HPP

namespace nb
{
    namespace Templates
    {
        template <typename Derived, typename T>
        class IBinaryTree
        {
            public:
                
                IBinaryTree() = default;

                ~BIinaryTree() = default;
                
                void insert(const T& val)
                {
                    dynamic_cast<Derived *>(this)->insert(val);
                }

                void remove()
                {
                    dynamic_cast<Derived *>(this)->remove();
                }

                Node* find(const T& val)
                {

                }

                // travel
                void preOrder()
                {

                }

                void inOrder()
                {

                }

                void postOrder()
                {

                }

                void levelOrder()
                {

                }
                //

                inline constexpr uint8_t depth() const noexcept 
                {
                     
                }


            protected:

                struct Node
                {
                    explicit Node(T data)
                        :data(data)
                    {

                    }
                    
                    T data = {};
                    Node* left = nullptr;
                    Node* right = nullptr;
                };

                Node *root = nullptr;
        };
    };
};

#endif