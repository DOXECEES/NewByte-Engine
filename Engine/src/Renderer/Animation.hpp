#pragma once
#include <algorithm> // Для std::find_if
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <cassert>    // Для assert
#include <filesystem> // Для std::filesystem
#include <string>
#include <unordered_map>
#include <vector>

#include "Bone.hpp"
#include "Mesh.hpp"
#include "RendererStructures.hpp"  // Содержит AssimpNodeData, BoneInfo и т.д.
#include "Resources/IResource.hpp" // Подключаем базовый класс ресурсов вашего движка

namespace nb::Renderer
{
    class Animation : public Resource::IResource
    {
    public:
        // Конструктор по умолчанию инициализирует базовый ресурс пустой строкой
        Animation() : Resource::IResource("")
        {
        }

        // Конструктор загрузки передает путь анимации в базовый класс IResource
        Animation(
            const std::string& animationPath,
            Ref<Mesh>          mesh,
            int                trackIndex = 0 // <- добавили
        )
            : Resource::IResource(animationPath)
        {
            Assimp::Importer importer;

            if (std::filesystem::path(animationPath).extension() == ".fbx")
            {
                importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f);
            }

            const aiScene* scene =
                importer.ReadFile(animationPath, aiProcess_Triangulate | aiProcess_GlobalScale);

            assert(scene && scene->mRootNode);

            if (scene->mNumAnimations == 0)
            {
                return;
            }

            // ИСПРАВЛЕНО: используем trackIndex, с защитой от выхода за границы
            int clampedIndex =
                (trackIndex >= 0 && trackIndex < (int)scene->mNumAnimations) ? trackIndex : 0;

            auto aiAnimation = scene->mAnimations[clampedIndex];
            m_Duration       = (float)aiAnimation->mDuration;
            m_TicksPerSecond =
                aiAnimation->mTicksPerSecond != 0 ? (float)aiAnimation->mTicksPerSecond : 24.0f;

            ReadHeirarchyData(m_RootNode, scene->mRootNode);
            ReadMissingBones(aiAnimation, *mesh);
        }

        // Статический метод для загрузки всех анимаций из одного файла
        static std::vector<std::shared_ptr<Animation>> LoadAllAnimations(
            const std::string& animationPath,
            Ref<Mesh>          mesh
        )
        {
            std::vector<std::shared_ptr<Animation>> animations;

            Assimp::Importer importer;

            if (std::filesystem::path(animationPath).extension() == ".fbx")
            {
                importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f);
            }

            const aiScene* scene =
                importer.ReadFile(animationPath, aiProcess_Triangulate | aiProcess_GlobalScale);

            if (!scene || scene->mNumAnimations == 0)
            {
                return animations;
            }

            for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
            {
                // Создаем ресурс и явно передаем путь в конструктор IResource
                auto anim = std::shared_ptr<Animation>(new Animation(animationPath));

                auto aiAnimation = scene->mAnimations[i];
                anim->m_Duration = (float)aiAnimation->mDuration;
                anim->m_TicksPerSecond =
                    aiAnimation->mTicksPerSecond != 0 ? (float)aiAnimation->mTicksPerSecond : 24.0f;

                anim->ReadHeirarchyData(anim->m_RootNode, scene->mRootNode);
                anim->ReadMissingBones(aiAnimation, *mesh);

                animations.push_back(anim);
            }

            return animations;
        }

        Bone* FindBone(const std::string& name)
        {
            auto iter = std::find_if(
                m_Bones.begin(), m_Bones.end(),
                [&](const Bone& bone)
                {
                    return bone.GetBoneName() == name;
                }
            );
            if (iter == m_Bones.end())
            {
                return nullptr;
            }
            return &(*iter);
        }

        float GetTicksPerSecond() const
        {
            return m_TicksPerSecond;
        }
        float GetDuration() const
        {
            return m_Duration;
        }
        const AssimpNodeData& GetRootNode() const
        {
            return m_RootNode;
        }
        const auto& GetBoneIDMap() const
        {
            return m_BoneInfoMap;
        }

    private:
        // Приватный конструктор специально для метода LoadAllAnimations
        Animation(const std::string& animationPath) : Resource::IResource(animationPath)
        {
        }

        void ReadMissingBones(
            const aiAnimation* animation,
            Mesh&              mesh
        )
        {
            int   size        = animation->mNumChannels;
            auto& boneInfoMap = mesh.GetBoneInfoMap();
            int&  boneCount   = mesh.GetBoneCount();

            // Читаем каналы анимации
            for (int i = 0; i < size; i++)
            {
                auto        channel  = animation->mChannels[i];
                std::string boneName = channel->mNodeName.data;

                if (boneInfoMap.find(boneName) == boneInfoMap.end())
                {
                    boneInfoMap[boneName].id = boneCount;
                    boneCount++;
                }
                m_Bones.push_back(
                    Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel)
                );
            }

            m_BoneInfoMap = boneInfoMap;
        }

        void ReadHeirarchyData(
            AssimpNodeData& dest,
            const aiNode*   src
        )
        {
            assert(src);

            dest.name           = src->mName.data;
            dest.transformation = ConvertMatrixToNbFormat(src->mTransformation);
            dest.childrenCount  = src->mNumChildren;

            for (unsigned int i = 0; i < src->mNumChildren; i++)
            {
                AssimpNodeData newData;
                ReadHeirarchyData(newData, src->mChildren[i]);
                dest.children.push_back(newData);
            }
        }

        // Вспомогательный хелпер транспонирования матриц Assimp -> Math::Mat4<float> (Column-Major)
        Math::Mat4<float> ConvertMatrixToNbFormat(const aiMatrix4x4& from)
        {
            Math::Mat4<float> to;
            to[0][0] = from.a1;
            to[1][0] = from.a2;
            to[2][0] = from.a3;
            to[3][0] = from.a4;
            to[0][1] = from.b1;
            to[1][1] = from.b2;
            to[2][1] = from.b3;
            to[3][1] = from.b4;
            to[0][2] = from.c1;
            to[1][2] = from.c2;
            to[2][2] = from.c3;
            to[3][2] = from.c4;
            to[0][3] = from.d1;
            to[1][3] = from.d2;
            to[2][3] = from.d3;
            to[3][3] = from.d4;
            return to;
        }

        float                                     m_Duration       = 0.0f;
        float                                     m_TicksPerSecond = 0.0f;
        std::vector<Bone>                         m_Bones;
        AssimpNodeData                            m_RootNode;
        std::unordered_map<std::string, BoneInfo> m_BoneInfoMap;
    };
} // namespace nb::Renderer