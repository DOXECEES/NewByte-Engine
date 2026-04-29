#include "Loaders/JSON/Json.hpp"
#include "Renderer/Mesh.hpp"
#include <fstream>
#include <vector>

namespace nb::Resource
{
    // Эта структура должна ТОЧНО совпадать с тем, что записывает AssetImporter
    struct RawVertex
    {
        float px, py, pz; // Position
        float nx, ny, nz; // Normal
        float tx, ty;     // UV
    };

    Ref<Renderer::Mesh> loadModel(const std::filesystem::path& modelPath)
    {
        // 1. ПАРСИНГ .model (JSON)
        // Используем твой класс обертку для JSON
        auto modelJson = nb::Loaders::Json(modelPath);

        //if (!modelJson.contains("mesh_source"))
        //{
        //    return nullptr;
        //}

        std::string           meshFileName = modelJson["mesh_source"].get<std::string>();
        std::filesystem::path meshFilePath = meshFileName;

        // 2. ОТКРЫТИЕ БИНАРНОГО .mesh
        std::ifstream meshF(meshFilePath, std::ios::binary);
        if (!meshF.is_open())
        {
            return nullptr;
        }

        uint32_t numSubmeshes = 0;
        meshF.read((char*)&numSubmeshes, sizeof(uint32_t));

        std::vector<Renderer::Vertex>                   allVertices;
        std::vector<std::unique_ptr<Renderer::SubMesh>> submeshes;

        uint32_t vertexOffset = 0;

        // 3. ЦИКЛ ПО САБМЕШАМ
        for (uint32_t i = 0; i < numSubmeshes; ++i)
        {
            uint32_t vCount, iCount;
            meshF.read((char*)&vCount, sizeof(uint32_t));
            meshF.read((char*)&iCount, sizeof(uint32_t));

            // --- ИСПРАВЛЕНИЕ ВЫЛЕТА ---
            // Читаем сырые данные во временный буфер нужного размера
            std::vector<RawVertex> rawData(vCount);
            meshF.read((char*)rawData.data(), sizeof(RawVertex) * vCount);

            // Резервируем место в основном пуле и перекладываем данные
            size_t startIdx = allVertices.size();
            allVertices.resize(startIdx + vCount);

            for (uint32_t v = 0; v < vCount; ++v)
            {
                auto&       target = allVertices[startIdx + v];
                const auto& source = rawData[v];

                // Копируем только те поля, что есть в файле
                target.position           = {source.px, source.py, source.pz};
                target.normal             = {source.nx, source.ny, source.nz};
                target.textureCoordinates = {source.tx, source.ty};
                // Поле tangent (Vector4) остается неинициализированным,
                // оно рассчитается в конструкторе Mesh ниже.
            }
            // --------------------------

            // Загружаем индексы
            std::vector<uint32_t> subIndices(iCount);
            meshF.read((char*)subIndices.data(), sizeof(uint32_t) * iCount);

            // Корректируем индексы под общий буфер вершин
            for (auto& idx : subIndices)
            {
                idx += vertexOffset;
            }

            // Загружаем материал
            Renderer::Material mat;
            // Пытаемся найти материал для текущего сабмеша в JSON
            //if (modelJson.contains("submeshes") && i < modelJson["submeshes"].size())
            //{
            //    // Тут можно добавить загрузку MaterialAsset через ResourceManager,
            //    // если в SubMesh нужна ссылка на ресурс.
            //    // Пока оставляем пустой или дефолтный.
            //}

            // Создаем сабмеш
            submeshes.push_back(std::make_unique<Renderer::SubMesh>(subIndices, mat));

            vertexOffset += vCount;
        }

        meshF.close();

        // 4. СОЗДАНИЕ ОБЪЕКТА MESH
        // В конструкторе Mesh автоматически вызовется:
        // - uniteIndicies()
        // - calculateTagnentArray() (рассчитает недостающие тангенты)
        // - VAO.linkData()
        auto meshAsset = std::make_shared<Renderer::Mesh>(
            std::move(submeshes), std::move(allVertices), modelPath
        );

        return meshAsset;
    }
} // namespace nb::Resource