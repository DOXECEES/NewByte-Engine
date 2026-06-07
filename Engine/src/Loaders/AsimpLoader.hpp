#pragma once
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <memory>
#include <vector>
#include <unordered_map>
#include <cassert>

#include "Renderer/Mesh.hpp"
#include "Renderer/RendererStructures.hpp"

namespace nb::Renderer
{
    // Вспомогательный метод конвертации матриц Assimp -> Math::Mat4<float>
    inline Math::Mat4<float> ConvertMatrixToNbFormat(const aiMatrix4x4& from)
    {
        Math::Mat4<float> to;
        // Обратите внимание на транспонирование (Assimp использует Row-Major, OpenGL — Column-Major)
        to[0][0] = from.a1; to[1][0] = from.a2; to[2][0] = from.a3; to[3][0] = from.a4;
        to[0][1] = from.b1; to[1][1] = from.b2; to[2][1] = from.b3; to[3][1] = from.b4;
        to[0][2] = from.c1; to[1][2] = from.c2; to[2][2] = from.c3; to[3][2] = from.c4;
        to[0][3] = from.d1; to[1][3] = from.d2; to[2][3] = from.d3; to[3][3] = from.d4;
        return to;
    }

    class AssimpLoader
    {
    public:
        // Параметр isAnimated управляет тем, отключаем ли мы пред-трансформацию вершин
        static std::shared_ptr<Mesh> load(const std::filesystem::path& path, bool isAnimated = false)
        {
            Assimp::Importer importer;

            if (path.extension() == ".fbx")
            {
                importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f);
            }
            else
            {
                importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);
            }

            uint32_t flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                             aiProcess_OptimizeMeshes  |  
                             aiProcess_FlipUVs | 
                             aiProcess_GlobalScale | aiProcess_CalcTangentSpace |
                             aiProcess_JoinIdenticalVertices;

            // Если модель НЕ анимированная — сжимаем иерархию для оптимизации
            if (!isAnimated)
            {
                flags |= aiProcess_PreTransformVertices;
            }

            const aiScene* aiScene = importer.ReadFile(path.string(), flags);

            if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode)
            {
                return nullptr;
            }

            std::vector<Vertex>                   allVertices;
            std::vector<std::unique_ptr<SubMesh>> subMeshes;

            std::unordered_map<std::string, BoneInfo> boneInfoMap;
            int boneCount = 0;

            for (uint32_t i = 0; i < aiScene->mNumMeshes; i++)
            {
                aiMesh* aiMesh = aiScene->mMeshes[i];
                subMeshes.push_back(processSubMesh(aiMesh, aiScene, allVertices, boneInfoMap, boneCount, isAnimated));
            }

            auto mesh = std::make_shared<Mesh>(std::move(subMeshes), std::move(allVertices), path);
            
            // Сохраняем информацию о костях в созданный меш
            mesh->boneInfoMap = std::move(boneInfoMap);
            mesh->boneCount = boneCount;

            return mesh;
        }

    private:
        static void SetVertexBoneData(Vertex& vertex, int boneID, float weight)
        {
            for (int i = 0; i < 4; ++i)
            {
                if (vertex.boneIDs[i] < 0)
                {
                    vertex.boneIDs[i] = boneID;
                    vertex.weights[i] = weight;
                    return;
                }
            }
        }

        static void ExtractBoneWeightsForVertices(
            std::vector<Vertex>& vertices, 
            uint32_t vertexOffset, 
            aiMesh* mesh, 
            std::unordered_map<std::string, BoneInfo>& boneInfoMap, 
            int& boneCount)
        {
            for (uint32_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
            {
                aiBone* bone = mesh->mBones[boneIndex];
                std::string boneName = bone->mName.C_Str();
                int boneID = -1;

                if (boneInfoMap.find(boneName) == boneInfoMap.end())
                {
                    BoneInfo newBoneInfo;
                    newBoneInfo.id = boneCount;
                    newBoneInfo.offsetMatrix = ConvertMatrixToNbFormat(bone->mOffsetMatrix);
                    boneInfoMap[boneName] = newBoneInfo;
                    boneID = boneCount;
                    boneCount++;
                }
                else
                {
                    boneID = boneInfoMap[boneName].id;
                }

                assert(boneID != -1);
                auto weights = bone->mWeights;
                uint32_t numWeights = bone->mNumWeights;

                for (uint32_t weightIndex = 0; weightIndex < numWeights; ++weightIndex)
                {
                    uint32_t vertexId = vertexOffset + weights[weightIndex].mVertexId;
                    float weight = weights[weightIndex].mWeight;
                    assert(vertexId < vertices.size());
                    SetVertexBoneData(vertices[vertexId], boneID, weight);
                }
            }
        }

        static std::unique_ptr<SubMesh> processSubMesh(
            aiMesh*              mesh,
            const aiScene*       scene,
            std::vector<Vertex>& allVertices,
            std::unordered_map<std::string, BoneInfo>& boneInfoMap,
            int& boneCount,
            bool isAnimated
        )
        {
            std::vector<uint32_t> subIndices;
            uint32_t              vertexOffset = static_cast<uint32_t>(allVertices.size());

            // 1. Загрузка геометрии вершин
            for (uint32_t i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;

                vertex.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
                vertex.normal   = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };

                if (mesh->mTextureCoords[0])
                {
                    vertex.textureCoordinates = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y };
                }
                else
                {
                    vertex.textureCoordinates = { 0.0f, 0.0f };
                }

                if (mesh->HasTangentsAndBitangents())
                {
                    vertex.tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 1.0f };
                }

                allVertices.push_back(vertex);
            }

            // 2. Извлечение скелетных весов (только если анимация включена)
            if (isAnimated && mesh->HasBones())
            {
                ExtractBoneWeightsForVertices(allVertices, vertexOffset, mesh, boneInfoMap, boneCount);
            }

            // 3. Загрузка индексов
            for (uint32_t i = 0; i < mesh->mNumFaces; i++)
            {
                aiFace face = mesh->mFaces[i];
                for (uint32_t j = 0; j < face.mNumIndices; j++)
                {
                    subIndices.push_back(vertexOffset + face.mIndices[j]);
                }
            }

            Material mat;
            return std::make_unique<SubMesh>(subIndices, mat);
        }
    };
} // namespace nb::Renderer