#ifndef NBSTL_SRC_NTREE_HPP
#define NBSTL_SRC_NTREE_HPP

#include <array>
#include <memory>
#include <stack>
#include <queue>
#include <optional>

namespace nbstl
{

    template <typename T, size_t Count, typename Node>
    class NTree;

    template<size_t CountOfChild>
	class TreeNode : public std::enable_shared_from_this<TreeNode<CountOfChild>>

	{
	public:

		explicit TreeNode() = delete;
        explicit TreeNode(const std::shared_ptr<TreeNode>& parent) 
            : parent(parent)
        {}

        virtual ~TreeNode() noexcept = default;

        std::shared_ptr<TreeNode> getChild(const size_t index) const noexcept
        {
            return CountOfChild > index ? childs[index] : nullptr;
        }

		const std::array<std::shared_ptr<TreeNode>, CountOfChild>& getChilds() const noexcept
		{
			return childs;
		}

        void setChild(const size_t index, const std::shared_ptr<TreeNode>& child) noexcept
        {
            if (index < CountOfChild)
			{
				childs[index] = child;
				if (child)
				{
					child->parent = this->weak_from_this();
				}
			}

        }

        void setChilds(const std::array<std::shared_ptr<TreeNode>, CountOfChild>& newChilds) noexcept
        {
            this->childs = newChilds;
			for (auto& child : this->childs)
			{
				if (child) 
				{
					child->parent = this->weak_from_this();
				}
    		}

        }

        std::optional<size_t> indexOf(const std::shared_ptr<TreeNode>& node) const noexcept
        {
            for(size_t i = 0; i < CountOfChild; i++)
            {
                if(childs[i] == node)
                {
                    return i;
                }
            }
            return std::nullopt;
        }

        void setParent(const std::shared_ptr<TreeNode>& parent) noexcept
        {
            this->parent = parent;
        }

        std::shared_ptr<TreeNode> getParent() const noexcept
        {
            return parent.lock();
        }

        size_t getCurrentCountOfChilds() const noexcept
        {
            size_t count = 0;
            for (size_t i = 0; i < CountOfChild; i++)
            {
                if(childs[i])
                    count++;
            }
            return count;
        }
        
        size_t getCountOfChilds() const noexcept
        {
            return CountOfChild;
        }

	protected:

        std::weak_ptr<TreeNode>                             parent = nullptr;

		std::array<std::shared_ptr<TreeNode>, CountOfChild> childs;
        
        template<typename T, size_t Count, typename Node>
        friend class NTree;

	};

	template <typename T, size_t  Count, typename Node = TreeNode<Count>>
	class NTree
	{
	public:

		explicit NTree()				noexcept = delete;

		explicit NTree(const std::shared_ptr<Node>& data)	noexcept
			: root(data)
		{}

		virtual ~NTree() noexcept = default;

		virtual std::shared_ptr<Node> insert(const std::shared_ptr<Node>& parent, const std::shared_ptr<Node>& insertNode)							noexcept = 0;
		virtual void insert(const std::shared_ptr<Node>& parent, const std::shared_ptr<Node>& insertNode, const size_t position)	noexcept = 0;

		virtual void removeNodeRecursively(const std::shared_ptr<Node>& removeNode) noexcept = 0;
		virtual void removeNode(const std::shared_ptr<Node>& removeNode)			noexcept = 0;

		virtual std::shared_ptr<Node> find(const std::shared_ptr<Node>& lookupNode) const noexcept = 0;
		
		//template<typename Predicate>
		//virtual std::shared_ptr<Node>& find(Predicate predicate) const noexcept = 0;

		virtual void inOrderTraversal()		noexcept = 0;
		virtual void preOrderTraversal()	noexcept = 0;
		virtual void postOrderTraversal()	noexcept = 0;
		virtual void levelOrderTraversal()	noexcept = 0;

		const std::array<std::shared_ptr<Node>, Count>& getChilds(const std::shared_ptr<Node>& parent) const noexcept
		{
			return parent->getChilds();
		}

		bool isLeaf(const std::shared_ptr<Node>& node) const noexcept
		{
			const std::array<std::shared_ptr<Node>, Count>& childs = node->getChilds();
			for (size_t i = 0; i < Count; i++)
			{
				if (childs[i]) return false;
			}

			return true;
		}

		size_t getHeight() const noexcept
		{
			return getHeight(root);
		}

		size_t getHeight(const std::shared_ptr<Node>& treeNode) const noexcept
		{
			if (!treeNode)
			{
				return 0;
			}

			std::queue<std::shared_ptr<Node>> q;
			q.push(root);
			size_t depth = 0;

			while (!q.empty())
			{
				int levelSize = q.size();

				for (int i = 0; i < levelSize; i++)
				{
					std::shared_ptr<Node> curr = q.front();
					q.pop();
					for (const auto& child : getChilds(curr))
					{
						if (child)
						{
							q.push(child);
						}
					}
				}

				depth++;
			}

			return depth - 1;
		}

        static void replaceNode(const std::shared_ptr<Node>& oldNode, const std::shared_ptr<Node>& newNode) noexcept
        {
            
            std::shared_ptr<Node> parentNode = std::dynamic_pointer_cast<Node>(oldNode->parent.lock());
            
            if(!parentNode) 
            {
                newNode->parent.reset();
                newNode->setChilds(oldNode->getChilds());
                return;
            }
            
            if(auto index = parentNode->indexOf(oldNode))
            {
                parentNode->setChild(*index, newNode);
            }

            newNode->parent = parentNode;

            newNode->setChilds(oldNode->getChilds());
        }


		std::shared_ptr<Node>	root = nullptr;

	};

};

#endif