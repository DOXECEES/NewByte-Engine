#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include "Math/AABB3D.hpp"
#include "Math/Vector3.hpp"

namespace nb::Math
{
    struct BVHNode
    {
        AABB3D bounds;
        uint32_t leftFirst;
        uint32_t count;

        bool isLeaf() const
        {
            return count > 0;
        }
    };

    struct BVHItem
    {
        uint32_t entityId;
        AABB3D worldAABB;
    };

    class BVH
    {
    public:
        std::vector<BVHNode> nodes;
        std::vector<BVHItem> items;

        void build(std::vector<BVHItem>&& inputItems)
        {
            items = std::move(inputItems);
            nodes.clear();
            if (items.empty())
            {
                return;
            }

            // Резервируем память (2N - 1)
            nodes.reserve(items.size() * 2);

            BVHNode root;
            root.leftFirst = 0;
            root.count = static_cast<uint32_t>(items.size());
            nodes.push_back(root);

            updateNodeBounds(0);
            subdivide(0);
        }

    private:
        void updateNodeBounds(uint32_t nodeIdx)
        {
            BVHNode& node = nodes[nodeIdx];
            node.bounds.setInvalid(); // ОБЯЗАТЕЛЬНО: сбрасываем min/max в +/- infinity

            if (node.isLeaf())
            {
                for (uint32_t i = 0; i < node.count; i++)
                {
                    node.bounds.grow(items[node.leftFirst + i].worldAABB);
                }
            }
            else
            {
                // Для внутренних узлов объединяем границы детей
                node.bounds.grow(nodes[node.leftFirst].bounds);
                node.bounds.grow(nodes[node.leftFirst + 1].bounds);
            }
        }

        void subdivide(uint32_t nodeIdx)
        {
            // Работаем с локальными копиями данных, так как вектор может перераспределиться
            uint32_t first = nodes[nodeIdx].leftFirst;
            uint32_t count = nodes[nodeIdx].count;

            if (count <= 1)
            {
                return;
            }

            // 1. Выбор оси (самая длинная)
            Vector3<float> extent = nodes[nodeIdx].bounds.size();
            int axis = 0;
            if (extent.y > extent.x)
            {
                axis = 1;
            }
            if (extent.z > (axis == 1 ? extent.y : extent.x))
            {
                axis = 2;
            }

            // 2. Расчет позиции разделения (Geometric Split)
            float splitPos = nodes[nodeIdx].bounds.minPoint[axis] + extent[axis] * 0.5f;

            // 3. Partition
            int i = first;
            int j = first + count - 1;
            while (i <= j)
            {
                if (items[i].worldAABB.center()[axis] < splitPos)
                {
                    i++;
                }
                else
                {
                    std::swap(items[i], items[j--]);
                }
            }

            // 4. Проверка: удалось ли разделить?
            uint32_t leftCount = i - first;
            if (leftCount == 0 || leftCount == count)
            {
                // Если все центры объектов оказались с одной стороны от splitPos,
                // делим массив просто пополам (Median Split), чтобы дерево не схлопнулось.
                i = first + count / 2;
                leftCount = i - first;
            }

            // 5. Создаем детей
            uint32_t leftChildIdx = static_cast<uint32_t>(nodes.size());
            nodes.emplace_back(); // Ребенок 1
            nodes.emplace_back(); // Ребенок 2

            // Инициализируем детей (они пока листья)
            nodes[leftChildIdx].leftFirst = first;
            nodes[leftChildIdx].count = leftCount;

            nodes[leftChildIdx + 1].leftFirst = i;
            nodes[leftChildIdx + 1].count = count - leftCount;

            // 6. Превращаем текущий узел во внутренний
            // Важно: получаем доступ к nodes[nodeIdx] ПОВТОРНО, т.к. emplace_back мог
            // инвалидировать ссылку
            nodes[nodeIdx].leftFirst = leftChildIdx;
            nodes[nodeIdx].count = 0;

            // 7. Обновляем границы детей
            updateNodeBounds(leftChildIdx);
            updateNodeBounds(leftChildIdx + 1);

            // 8. Рекурсия
            subdivide(leftChildIdx);
            subdivide(leftChildIdx + 1);
        }
    };
} // namespace nb::Math