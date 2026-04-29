#pragma once
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <filesystem>
#include <memory>
#include <vector>

#include "Renderer/Mesh.hpp"
#include "Renderer/RendererStructures.hpp"

namespace nb::Renderer
{
    class AssimpLoader
    {
    public:
        static std::shared_ptr<Mesh> load(const std::filesystem::path& path)
        {
            Assimp::Importer importer;

            // Настройка: если Blender экспортирует в см, а вам нужны метры.
            // 0.01 уменьшит модель в 100 раз. Если размер в Blender устраивает, поставьте 1.0f
            if (path.extension() == ".fbx")
            {
                importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 0.01f);
            }
            else
            {
                importer.SetPropertyFloat(AI_CONFIG_GLOBAL_SCALE_FACTOR_KEY, 1.0f);
            }
            // КРИТИЧЕСКИЕ ФЛАГИ:
            // 1. aiProcess_PreTransformVertices - Assimp сам применит все матрицы узлов.
            //    Это решит проблему сплющивания, так как иерархия будет "схлопнута" правильно.
            // 2. aiProcess_GlobalScale - применит коэффициент масштаба выше.
            // 3. aiProcess_MakeLeftHanded - если ваш движок Row-Major (DirectX стиль),
            //    часто требуется левосторонняя система.
            uint32_t flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                             aiProcess_OptimizeMeshes  |  
                             aiProcess_FlipUVs | aiProcess_PreTransformVertices |
                             aiProcess_GlobalScale | aiProcess_CalcTangentSpace |
                             aiProcess_JoinIdenticalVertices;

            const aiScene* aiScene = importer.ReadFile(path.string(), flags);

            if (!aiScene || aiScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !aiScene->mRootNode)
            {
                return nullptr;
            }

            std::vector<Vertex>                   allVertices;
            std::vector<std::unique_ptr<SubMesh>> subMeshes;


            for (uint32_t i = 0; i < aiScene->mNumMeshes; i++)
            {
                aiMesh* aiMesh = aiScene->mMeshes[i];
                subMeshes.push_back(processSubMesh(aiMesh, aiScene, allVertices));
            }

            return std::make_shared<Mesh>(std::move(subMeshes), std::move(allVertices), path);
        }

    private:
        static std::unique_ptr<SubMesh> processSubMesh(
            aiMesh*              mesh,
            const aiScene*       scene,
            std::vector<Vertex>& allVertices
        )
        {
            std::vector<uint32_t> subIndices;
            uint32_t              vertexOffset = static_cast<uint32_t>(allVertices.size());

            for (uint32_t i = 0; i < mesh->mNumVertices; i++)
            {
                Vertex vertex;

                // С флагом PreTransformVertices координаты уже финальные.
                // Просто копируем их.
                vertex.position = {
                    mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z
                };
                vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};

                if (mesh->mTextureCoords[0])
                {
                    vertex.textureCoordinates = {
                        mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y
                    };
                }
                else
                {
                    vertex.textureCoordinates = {0.0f, 0.0f};
                }

                if (mesh->HasTangentsAndBitangents())
                {
                    vertex.tangent = {
                        mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 1.0f
                    };
                }

                allVertices.push_back(vertex);
            }

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