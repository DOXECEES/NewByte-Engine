#ifndef SRC_SHARED_HUFFMANTREE_HPP
#define SRC_SHARED_HUFFMANTREE_HPP

#include "../Templates/BinaryTree.hpp"

namespace nb
{
    namespace Shared
    {
        template<typename T>
        class HuffmanTree : public nb::Templates::IBinaryTree<HuffmanTree, T> 
        {
            public:
                HuffmanTree() = default;

            
                void insert();
                void remove();

                void build();


            private:
                                


        };
    };
};

#endif