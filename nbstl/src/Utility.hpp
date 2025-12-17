#ifndef NBSTL_SRC_UTILITY_HPP
#define NBSTL_SRC_UTILITY_HPP

#include <Vector.hpp>

namespace nbstl 
{

    template<
        typename Node,
        typename GetChildrenFunc,
        typename Callback
    >
    bool dfs(const Node* root, GetChildrenFunc&& getChildren, Callback&& callback)
    {
        nbstl::Vector<const Node*> stack;
        stack.pushBack(root);

        while (!stack.isEmpty())
        {
            const Node* node = stack.back();
            stack.popBack();

            if (callback(node))
            {
                return true;
            }

            auto children = getChildren(node);
            for (int i = static_cast<int>(children.size()) - 1; i >= 0; i--)
            {
                stack.pushBack(children[i]);
            }
        }

        return false;
    }




};

#endif 