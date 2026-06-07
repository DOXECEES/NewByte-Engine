#include "Loaders/JSON/Json.hpp"
#include "Renderer/Mesh.hpp"
#include <fstream>
#include <vector>

namespace nb::Resource
{
    // Новый формат вершин с костями (64 байта)
    struct RawVertex
    {
        float px, py, pz;
        float nx, ny, nz;
        float tx, ty;
        int32_t boneIDs[4];
        float   weights[4];
    };

    // Старый формат вершин без костей (32 байта) для обратной совместимости
    struct OldRawVertex
    {
        float px, py, pz;
        float nx, ny, nz;
        float tx, ty;
    };

    Ref<Renderer::Mesh> loadModel(const std::filesystem::path& modelPath)
    {
        // 1. ПАРСИНГ .model (JSON)
        auto modelJson = nb::Loaders::Json(modelPath);

        std::string           meshFileName = modelJson["mesh_source"].get<std::string>();
        std::filesystem::path meshFilePath = meshFileName;

        // 2. ОТКРЫТИЕ БИНАРНОГО .mesh
        std::ifstream meshF(meshFilePath, std::ios::binary);
        if (!meshF.is_open())
        {
            return nullptr;
        }

        // Проверяем, содержит ли модель данные костей
        bool hasBonesData = modelJson.contains("bones");

        uint32_t numSubmeshes = 0;
        meshF.read((char*)&numSubmeshes, sizeof(uint32_t));

        std::vector<Renderer::Vertex>                   allVertices;
        std::vector<std::unique_ptr<Renderer::SubMesh>> submeshes;

        uint32_t vertexOffset = 0;

        // 3. ЦИКЛ ПО САБМЕШАМ

        std::string ssdata = std::format(
            "SIZE TEST | sizeof(Vertex): {} bytes | sizeof(RawVertex): {} bytes",
            sizeof(nb::Renderer::Vertex), sizeof(RawVertex)
        );

        nb::Error::ErrorManager::instance().report(nb::Error::Type::FATAL, ssdata
        );

        for (uint32_t i = 0; i < numSubmeshes; ++i)
        {
            uint32_t vCount, iCount;
            meshF.read((char*)&vCount, sizeof(uint32_t));
            meshF.read((char*)&iCount, sizeof(uint32_t));

            size_t startIdx = allVertices.size();
            allVertices.resize(startIdx + vCount);

            if (hasBonesData)
            {
                // Читаем новый формат вершин (с костями)
                std::vector<RawVertex> rawData(vCount);
                meshF.read((char*)rawData.data(), sizeof(RawVertex) * vCount);

                for (uint32_t v = 0; v < vCount; ++v)
                {
                    auto&       target = allVertices[startIdx + v];
                    const auto& source = rawData[v];

                    target.position           = {source.px, source.py, source.pz};
                    target.normal             = {source.nx, source.ny, source.nz};
                    target.textureCoordinates = {source.tx, source.ty};
                    
                    target.boneIDs            = {source.boneIDs[0], source.boneIDs[1], source.boneIDs[2], source.boneIDs[3]};
                    target.weights            = {source.weights[0], source.weights[1], source.weights[2], source.weights[3]};
                }
            }
            else
            {
                // ОБРАТНАЯ СОВМЕСТИМОСТЬ: Читаем старый формат вершин (без костей)
                std::vector<OldRawVertex> rawData(vCount);
                meshF.read((char*)rawData.data(), sizeof(OldRawVertex) * vCount);

                for (uint32_t v = 0; v < vCount; ++v)
                {
                    auto&       target = allVertices[startIdx + v];
                    const auto& source = rawData[v];

                    target.position           = {source.px, source.py, source.pz};
                    target.normal             = {source.nx, source.ny, source.nz};
                    target.textureCoordinates = {source.tx, source.ty};
                    
                    // Заполняем дефолтными значениями для старых моделей
                    target.boneIDs            = {-1, -1, -1, -1};
                    target.weights            = {0.0f, 0.0f, 0.0f, 0.0f};


                }
            }

            std::vector<uint32_t> subIndices(iCount);
            meshF.read((char*)subIndices.data(), sizeof(uint32_t) * iCount);

            for (auto& idx : subIndices)
            {
                idx += vertexOffset;
            }

            Renderer::Material mat;
            submeshes.push_back(std::make_unique<Renderer::SubMesh>(subIndices, mat));

            vertexOffset += vCount;
        }
        meshF.close();

        auto meshAsset = std::make_shared<Renderer::Mesh>(
            std::move(submeshes), std::move(allVertices), modelPath
        );

        // ЧТЕНИЕ КАРТЫ КОСТЕЙ ИЗ JSON (только если они есть)
        if (hasBonesData)
        {
            auto& boneInfoMap = meshAsset->GetBoneInfoMap();
            int&  boneCount   = meshAsset->GetBoneCount();

            auto bonesList = modelJson["bones"];

            // Масштаб должен совпадать с тем что применяет Assimp GlobalScale (0.01 для FBX)
            // Если меш был экспортирован в cm и анимация грузится с GlobalScale=0.01,
            // то offsetMatrix тоже нужно привести к метрам
            const float OFFSET_SCALE = 0.01f; // <- применяем тот же коэффициент

            for (size_t i = 0; i < bonesList.size(); ++i)
            {
                auto const& bJson    = modelJson["bones"][i];
                std::string boneName = bJson["name"].get<std::string>();
                int         boneID   = bJson["id"].get<int>();

                auto offsetArrayNode = bJson["offset"];

                Math::Mat4<float> offsetMatrix;
                int               idx = 0;
                for (int row = 0; row < 4; ++row)
                {
                    for (int col = 0; col < 4; ++col)
                    {
                        offsetMatrix[row][col] = offsetArrayNode[idx++].get<float>();
                    }
                }

                // Масштабируем трансляционную часть offsetMatrix
                offsetMatrix[3][0] *= OFFSET_SCALE;
                offsetMatrix[3][1] *= OFFSET_SCALE;
                offsetMatrix[3][2] *= OFFSET_SCALE;

                Renderer::BoneInfo info{boneID, offsetMatrix};
                boneInfoMap[boneName] = info;
                boneCount             = std::max(boneCount, boneID + 1);
            }
        }

        return meshAsset;
    }
} // namespace nb::Resource