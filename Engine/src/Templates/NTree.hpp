// #ifndef SRC_TEMPLATES_NTREE_HPP
// #define SRC_TEMPLATES_NTREE_HPP

// #include <array>
// #include <memory>
// #include <stack>
// #include <queue>

// namespace nb
// {
// 	namespace Templates
// 	{
// 		template<typename T, size_t CountOfChild>
// 		class TreeNode
// 		{
// 		public:

// 			using valueType = T;

// 			explicit Node(const T& data)
// 				: data(data)
// 			{
// 			}

// 			void setData(const T& newData) noexcept
// 			{
// 				data = newData;
// 			}
		
// 			const std::array<std::shared_ptr<TreeNode>, CountOfChild>& getChilds() const noexcept
// 			{
// 				return childs;
// 			}

// 		protected:

// 			T data;

// 			std::array<std::shared_ptr<TreeNode>, CountOfChild> childs;

// 		};

// 		template <typename T, size_t  Count, typename Node = TreeNode<T, Count>>
// 		class NTree
// 		{
// 		public:

// 			explicit NTree() noexcept = delete;
// 			explicit NTree(const T& data) noexcept
// 				: root(std::make_shared(data))
// 			{}

// 			virtual ~NTree() noexcept = default;

// 			virtual void insert(const std::shared_ptr<Node>& parent, const std::shared_ptr<Node>& insertNode) noexcept = 0;
// 			virtual void insert(const std::shared_ptr<Node>& parent, const std::shared_ptr<Node>& insertNode, const size_t position) noexcept = 0;

// 			virtual void removeNodeRecursively(const std::shared_ptr<Node>& removeNode) noexcept = 0;
// 			virtual void removeNode(const std::shared_ptr<Node>& removeNode) noexcept = 0;

// 			virtual std::shared_ptr<Node>& find(const T& value) const noexcept = 0;
			
// 			template<typename Predicate>
// 			//template<typename Predicate>
// 			//virtual std::shared_ptr<Node>& find(const Predicate& predicate) const noexcept = 0;

// 			virtual void inOrderTraversal() noexcept = 0;
// 			virtual void preOrderTraversal() noexcept = 0;
// 			virtual void postOrderTraversal() noexcept = 0;
// 			virtual void levelOrderTraversal() noexcept = 0;

// 			const std::array<std::shared_ptr<Node>, Count>& getChilds(const std::shared_ptr<Node>& parent) const noexcept
// 			{
// 				return parent->getChilds();
// 			}

// 			bool isLeaf(const std::shared_ptr<Node>& node) const noexcept
// 			{
// 				const std::array<std::shared_ptr<Node>, Count>& childs = node->getChilds();
// 				for (size_t i = 0; i < Count; i++)
// 				{
// 					if (childs[i]) return false;
// 				}

// 				return true;
// 			}

// 			size_t getHeight() const noexcept
// 			{
// 				return getHeight(root);
// 			}

// 			size_t getHeight(const std::shared_ptr<Node>& treeNode) const noexcept
// 			{
// 				if (!treeNode)
// 				{
// 					return 0;
// 				}

// 				std::queue<std::shared_ptr<Node>> q;
// 				q.push(root);
// 				size_t depth = 0;

// 				while (!q.empty())
// 				{
// 					int levelSize = q.size();

// 					for (int i = 0; i < levelSize; i++)
// 					{
// 						std::shared_ptr<Node> curr = q.front();
// 						q.pop();
// 						for (const auto& child : getChilds(curr))
// 						{
// 							if (child)
// 							{
// 								q.push(child);
// 							}
// 						}
// 					}

// 					depth++;
// 				}

// 				return depth - 1;
// 			}


// 		private:

// 			std::shared_ptr<Node>	root;


// 		};
// 	};
// };

// #endif