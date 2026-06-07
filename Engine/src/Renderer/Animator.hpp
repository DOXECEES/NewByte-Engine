#pragma once
#include "Animation.hpp"
#include <algorithm> // Для std::max
#include <string>
#include <vector>

namespace nb::Renderer
{
    class Animator
    {
    public:
        Animator(Animation* animation)
        {
            m_CurrentTime      = 0.0;
            m_CurrentAnimation = animation;

            // Инициализируем вектор дефолтным размером 100
            m_FinalTransforms.reserve(100);
            for (int i = 0; i < 100; i++)
            {
                m_FinalTransforms.push_back(Math::Mat4<float>::identity());
            }
        }

        void UpdateAnimation(float dt)
        {
            if (m_CurrentAnimation)
            {
                // Рассчитываем текущее время в тиках
                m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
                m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());

                // Рассчитываем дерево костей рекурсивно
                CalculateBoneTransform(
                    &m_CurrentAnimation->GetRootNode(), Math::Mat4<float>::identity()
                );
            }
        }

        void PlayAnimation(Animation* pAnimation)
        {
            m_CurrentAnimation = pAnimation;
            m_CurrentTime      = 0.0f;
        }

        const std::vector<Math::Mat4<float>>& GetFinalBoneMatrices() const
        {
            return m_FinalTransforms;
        }

    private:
        void CalculateBoneTransform(
            const AssimpNodeData*    node,
            const Math::Mat4<float>& parentTransform
        )
        {
            std::string       nodeName      = node->name;
            Math::Mat4<float> nodeTransform = node->transformation;

            // Находим кость для текущего узла
            Bone* ActiveBone = m_CurrentAnimation->FindBone(nodeName);

            if (ActiveBone)
            {
                ActiveBone->Update(m_CurrentTime);
                nodeTransform = ActiveBone->GetLocalTransform();
            }

            // Глобальная трансформация узла: Глобальная матрица родителя * Локальная матрица узла
            Math::Mat4<float> globalTransformation = parentTransform * nodeTransform;

            // Если узел является костью нашего меша, применяем Offset Matrix (Inverse Bind Pose)
            auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
            if (boneInfoMap.find(nodeName) != boneInfoMap.end())
            {
                int               index  = boneInfoMap[nodeName].id;
                Math::Mat4<float> offset = boneInfoMap[nodeName].offsetMatrix;

                // ДИНАМИЧЕСКИЙ РАСШИРИТЕЛЬ:
                // Если индекс кости (например, 113) больше или равен размеру вектора,
                // плавно расширяем вектор, заполняя новые элементы единичными матрицами.
                if (index >= m_FinalTransforms.size())
                {
                    m_FinalTransforms.resize(index + 1, Math::Mat4<float>::identity());
                }

                // Теперь запись гарантированно безопасна
                m_FinalTransforms[index] = globalTransformation * offset;
            }

            // Рекурсивно обрабатываем детей
            for (int i = 0; i < node->childrenCount; i++)
            {
                CalculateBoneTransform(&node->children[i], globalTransformation);
            }
        }

        std::vector<Math::Mat4<float>> m_FinalTransforms;
        Animation*                     m_CurrentAnimation = nullptr;
        float                          m_CurrentTime      = 0.0f;
    };
} // namespace nb::Renderer