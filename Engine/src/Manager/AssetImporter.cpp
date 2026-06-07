#include "AssetImporter.hpp"
#include <Debug.hpp>
#include <algorithm>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "../../dependencies/stb/stb_image.h"
#include "../../dependencies/stb/stb_image_write.h"

namespace nb::SDK
{
    struct MeshVertex
    {
        float px, py, pz;
        float nx, ny, nz;
        float tx, ty;

        int32_t boneIDs[4];
        float   weights[4];
    };

    static std::string getAssetsRelativePath(const std::filesystem::path& fullPath)
    {
        std::string pathStr = fullPath.generic_string();
        size_t      pos     = pathStr.find("Assets/");
        if (pos != std::string::npos)
        {
            return pathStr.substr(pos);
        }
        return pathStr;
    }

    static std::string wstring_to_string(const std::wstring& wstr)
    {
        if (wstr.empty())
        {
            return {};
        }
        int size_needed =
            WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
        std::string strTo(size_needed, 0);
        WideCharToMultiByte(
            CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL
        );
        return strTo;
    }

    static std::wstring toW(const std::string& s)
    {
        return std::wstring(s.begin(), s.end());
    }

    static std::filesystem::path findTextureOnDisk(
        const std::filesystem::path& modelPath,
        const std::string&           filename
    )
    {
        auto                               dir = modelPath.parent_path();
        std::filesystem::path              p(filename);
        std::string                        pureName   = p.filename().string();
        std::vector<std::filesystem::path> candidates = {
            dir / pureName, dir / "textures" / pureName, dir / "Textures" / pureName
        };
        for (auto& cand : candidates)
        {
            if (std::filesystem::exists(cand))
            {
                return cand;
            }
        }
        return {};
    }

    // Безопасное чтение канала пикселя с масштабированием (ближайший сосед)
    static uint8_t sampleChannel(
        const unsigned char* data,
        int                  srcW,
        int                  srcH,
        int                  channels,
        int                  destX,
        int                  destY,
        int                  destW,
        int                  destH,
        int                  targetChannel
    )
    {
        if (!data)
        {
            return 0;
        }

        // Проекция координат из целевого разрешения в разрешение источника
        int x = (destX * srcW) / destW;
        int y = (destY * srcH) / destH;

        x = std::clamp(x, 0, srcW - 1);
        y = std::clamp(y, 0, srcH - 1);

        int index = (y * srcW + x) * channels;

        // Если запрошенного канала нет в файле (например, файл одноканальный), читаем 0-й канал
        int channelOffset = (targetChannel < channels) ? targetChannel : 0;
        return data[index + channelOffset];
    }

    // Функция сборки ORM карты из раздельных или совмещенных текстур
    static bool packORMTexture(
        const std::filesystem::path& aoPath,
        const std::filesystem::path& roughnessPath,
        const std::filesystem::path& metallicPath,
        const std::filesystem::path& outputPath
    ) noexcept
    {
        int aoW = 0, aoH = 0, aoC = 0;
        int roughW = 0, roughH = 0, roughC = 0;
        int metW = 0, metH = 0, metC = 0;

        unsigned char* aoData    = nullptr;
        unsigned char* roughData = nullptr;
        unsigned char* metData   = nullptr;

        // 1. Оптимизированная загрузка уникальных файлов (избегаем двойного чтения из диска)
        if (!aoPath.empty())
        {
            aoData = stbi_load(aoPath.string().c_str(), &aoW, &aoH, &aoC, 0);
        }

        if (!roughnessPath.empty())
        {
            if (roughnessPath == aoPath)
            {
                roughData = aoData;
                roughW    = aoW;
                roughH    = aoH;
                roughC    = aoC;
            }
            else
            {
                roughData = stbi_load(roughnessPath.string().c_str(), &roughW, &roughH, &roughC, 0);
            }
        }

        if (!metallicPath.empty())
        {
            if (metallicPath == aoPath)
            {
                metData = aoData;
                metW    = aoW;
                metH    = aoH;
                metC    = aoC;
            }
            else if (metallicPath == roughnessPath)
            {
                metData = roughData;
                metW    = roughW;
                metH    = roughH;
                metC    = roughC;
            }
            else
            {
                metData = stbi_load(metallicPath.string().c_str(), &metW, &metH, &metC, 0);
            }
        }

        // Если ни одна текстура не загрузилась, создавать файл бессмысленно
        if (!aoData && !roughData && !metData)
        {
            return false;
        }

        // 2. Определение целевых каналов выборки
        int aoSrcChannel    = 0; // Всегда красный
        int roughSrcChannel = 0; // По умолчанию красный (для раздельных файлов)
        int metSrcChannel   = 0; // По умолчанию красный (для раздельных файлов)

        // Если файлы шероховатости и металла совпадают (стандарт glTF Metallic-Roughness)
        if (!roughnessPath.empty() && roughnessPath == metallicPath)
        {
            roughSrcChannel = 1; // Зеленый канал (G) -> Roughness
            metSrcChannel   = 2; // Синий канал (B) -> Metallic
        }

        // Задаем результирующее разрешение по максимальной из валидных текстур
        int outW = std::max({aoW, roughW, metW});
        int outH = std::max({aoH, roughH, metH});

        std::vector<uint8_t> ormPixels(outW * outH * 3); // RGB формат

        for (int y = 0; y < outH; ++y)
        {
            for (int x = 0; x < outW; ++x)
            {
                int destIdx = (y * outW + x) * 3;

                // R: Ambient Occlusion (по умолчанию 255)
                ormPixels[destIdx + 0] =
                    aoData ? sampleChannel(aoData, aoW, aoH, aoC, x, y, outW, outH, aoSrcChannel)
                           : 255;

                // G: Roughness (по умолчанию 128)
                ormPixels[destIdx + 1] = roughData ? sampleChannel(
                                                         roughData, roughW, roughH, roughC, x, y,
                                                         outW, outH, roughSrcChannel
                                                     )
                                                   : 128;

                // B: Metallic (по умолчанию 0)
                ormPixels[destIdx + 2] =
                    metData
                        ? sampleChannel(metData, metW, metH, metC, x, y, outW, outH, metSrcChannel)
                        : 0;
            }
        }

        // Освобождаем только уникальные указатели на данные
        if (aoData)
        {
            stbi_image_free(aoData);
        }
        if (roughData && roughnessPath != aoPath)
        {
            stbi_image_free(roughData);
        }
        if (metData && metallicPath != roughnessPath && metallicPath != aoPath)
        {
            stbi_image_free(metData);
        }

        // Сохраняем готовую ORM-карту в формате PNG
        int success =
            stbi_write_png(outputPath.string().c_str(), outW, outH, 3, ormPixels.data(), outW * 3);
        return success != 0;
    }

    ImportResult AssetImporter::import(const ImportRequest& req) noexcept
    {
        std::string ext = req.sourceFile.extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == ".fbx" || ext == ".obj" || ext == ".gltf" || ext == ".glb")
        {
            return importModel(req);
        }

        if (ext == ".png" || ext == ".tga" || ext == ".jpg")
        {
            return importTexture(req.sourceFile, req.targetFolder);
        }

        return {false, L"Unsupported format"};
    }

    ImportResult AssetImporter::importTexture(
        const std::filesystem::path& src,
        const std::filesystem::path& dst
    ) noexcept
    {
        ImportResult result;
        ensureDir(dst);
        auto copied = copyFile(src, dst);
        if (copied.empty())
        {
            return {false, L"Copy error"};
        }

        std::filesystem::path texAsset = copied;
        texAsset.replace_extension(".texture");

        {
            std::ofstream f(texAsset);
            f << "{\n  \"source\": \"" << getAssetsRelativePath(copied)
              << "\",\n  \"should_flip\": true\n}";
        }
        result.success = true;
        result.assets.push_back({AssetType::TEXTURE, texAsset});
        return result;
    }

    ImportResult AssetImporter::importMaterial(
        const MaterialDesc&          mat,
        const std::filesystem::path& dst
    ) noexcept
    {
        ImportResult          result;
        std::filesystem::path matPath = dst / (mat.name + ".material");

        {
            std::ofstream f(matPath);
            f << "{\n";
            f << "  \"shader\": \"ADS.shader\",\n";
            f << "  \"properties\": {\n";

            auto writeTexProp = [&](const std::string&           propName,
                                    const std::filesystem::path& srcPath,
                                    const std::string& fallback, bool isLast)
            {
                std::string assetPath = fallback;

                if (!srcPath.empty() && std::filesystem::exists(srcPath))
                {
                    auto res = importTexture(srcPath, dst);
                    if (res.success)
                    {
                        assetPath = getAssetsRelativePath(res.assets[0].path);
                        result.assets.push_back(res.assets[0]);
                    }
                }

                f << "    \"" << propName << "\": \"" << assetPath << "\"" << (isLast ? "" : ",")
                  << "\n";
            };

            // Запись Albedo
            writeTexProp(
                "u_AlbedoMap", mat.textures.size() > 0 ? mat.textures[0] : "",
                "Assets/res/placeholder.texture", false
            );

            // Запись Normals
            writeTexProp(
                "u_NormalMap", mat.textures.size() > 1 ? mat.textures[1] : "",
                "Assets/res/normal_placeholder.texture", false
            );

            // === ЛОГИКА СБОРКИ ORM ===
            std::filesystem::path ormSourcePath;
            bool                  generatedORM = false;

            // Если у нас уже есть упакованная ORM-карта на индексе [2] и нет раздельных
            if (mat.textures.size() > 2 && !mat.textures[2].empty() && mat.textures[3].empty() &&
                mat.textures[4].empty())
            {
                ormSourcePath = mat.textures[2];
            }
            // Иначе, если у нас есть раздельные карты (или дублирующийся glTF файл в [3] и [4]), мы
            // их упаковываем
            else
            {
                std::filesystem::path ao    = mat.textures.size() > 2 ? mat.textures[2] : "";
                std::filesystem::path met   = mat.textures.size() > 3 ? mat.textures[3] : "";
                std::filesystem::path rough = mat.textures.size() > 4 ? mat.textures[4] : "";

                if (!ao.empty() || !met.empty() || !rough.empty())
                {
                    std::filesystem::path packedPngPath = dst / (mat.name + "_orm_packed.png");
                    if (packORMTexture(ao, rough, met, packedPngPath))
                    {
                        ormSourcePath = packedPngPath;
                        generatedORM  = true;
                    }
                }
            }

            writeTexProp("u_ORMMap", ormSourcePath, "Assets/res/placeholder.texture", false);

            if (generatedORM && std::filesystem::exists(ormSourcePath))
            {
                // При необходимости можно удалять временный сгенерированный файл:
                // std::filesystem::remove(ormSourcePath);
            }

            f << "    \"u_BaseColorFactor\": 1.0,\n";
            f << "    \"u_RoughnessFactor\": 0.5,\n";
            f << "    \"u_MetallicFactor\": 0.0,\n";
            f << "    \"u_OcclusionFactor\": 1.0,\n";
            f << "    \"u_Emission\": 0.0\n";
            f << "  }\n";
            f << "}";
        }

        result.success = true;
        result.assets.push_back({AssetType::MATERIAL, matPath});
        return result;
    }

    static std::string mat4ToJson(const aiMatrix4x4& m)
    {
        return std::format(
            "[{:.8f}, {:.8f}, {:.8f}, {:.8f},  {:.8f}, {:.8f}, {:.8f}, {:.8f},  {:.8f}, {:.8f}, "
            "{:.8f}, {:.8f},  {:.8f}, {:.8f}, {:.8f}, {:.8f}]",
            m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2, m.a3, m.b3, m.c3, m.d3, m.a4, m.b4,
            m.c4, m.d4
        );
    }

    ImportResult AssetImporter::importModel(const ImportRequest& req) noexcept
    {
        ImportResult result;
        result.success                    = true;
        std::filesystem::path assetFolder = req.targetFolder / req.assetName;
        ensureDir(assetFolder);

        Assimp::Importer importer;
        const aiScene*   scene = importer.ReadFile(
            req.sourceFile.string(), aiProcess_Triangulate | aiProcess_CalcTangentSpace |
                                         aiProcess_LimitBoneWeights | aiProcess_GenSmoothNormals
        );
        if (!scene)
        {
            return {false, L"Assimp Load Error"};
        }

        // --- Извлечение встроенных текстур на диск ---
        struct ExtractedTextures
        {
            std::vector<std::filesystem::path>                     byIndex;
            std::unordered_map<std::string, std::filesystem::path> byFilename;
        } extracted;

        if (scene->HasTextures())
        {
            extracted.byIndex.resize(scene->mNumTextures);
            for (uint32_t i = 0; i < scene->mNumTextures; ++i)
            {
                aiTexture*  tex          = scene->mTextures[i];
                std::string origFilename = tex->mFilename.length > 0 ? tex->mFilename.C_Str() : "";

                std::string ext = "png";
                if (tex->achFormatHint[0] != '\0')
                {
                    ext = tex->achFormatHint;
                    if (ext.front() == '.')
                    {
                        ext.erase(0, 1);
                    }
                }

                std::string pureName;
                if (!origFilename.empty())
                {
                    pureName = std::filesystem::path(origFilename).filename().string();
                }
                else
                {
                    pureName = wstring_to_string(req.assetName) + "_embedded_" + std::to_string(i) +
                               "." + ext;
                }

                std::filesystem::path outPath = assetFolder / pureName;

                if (tex->mHeight == 0) // Сжатый формат (PNG, JPEG и др.)
                {
                    std::ofstream out(outPath, std::ios::binary);
                    if (out)
                    {
                        out.write(reinterpret_cast<const char*>(tex->pcData), tex->mWidth);
                    }
                }
                else // Несжатый формат (ARGB8888) -> сохраняем как TGA
                {
                    outPath.replace_extension(".tga");
                    std::ofstream out(outPath, std::ios::binary);
                    if (out)
                    {
                        uint8_t header[18] = {0};
                        header[2]          = 2; // Uncompressed true-color
                        header[12]         = tex->mWidth & 0xFF;
                        header[13]         = (tex->mWidth >> 8) & 0xFF;
                        header[14]         = tex->mHeight & 0xFF;
                        header[15]         = (tex->mHeight >> 8) & 0xFF;
                        header[16]         = 32; // 32 bits per pixel (RGBA)

                        out.write(reinterpret_cast<const char*>(header), 18);

                        std::vector<uint8_t> pixelData(tex->mWidth * tex->mHeight * 4);
                        for (uint32_t pIdx = 0; pIdx < tex->mWidth * tex->mHeight; ++pIdx)
                        {
                            pixelData[pIdx * 4 + 0] = tex->pcData[pIdx].b;
                            pixelData[pIdx * 4 + 1] = tex->pcData[pIdx].g;
                            pixelData[pIdx * 4 + 2] = tex->pcData[pIdx].r;
                            pixelData[pIdx * 4 + 3] = tex->pcData[pIdx].a;
                        }
                        out.write(
                            reinterpret_cast<const char*>(pixelData.data()), pixelData.size()
                        );
                    }
                }

                extracted.byIndex[i] = outPath;
                if (!origFilename.empty())
                {
                    extracted.byFilename[origFilename] = outPath;
                    extracted.byFilename[pureName]     = outPath;
                }
            }
        }

        auto resolveTexturePath = [&](const std::string& pathStr) -> std::filesystem::path
        {
            if (pathStr.empty())
            {
                return {};
            }

            if (pathStr[0] == '*' && pathStr.size() > 1)
            {
                try
                {
                    int idx = std::stoi(pathStr.substr(1));
                    if (idx >= 0 && idx < static_cast<int>(extracted.byIndex.size()))
                    {
                        return extracted.byIndex[idx];
                    }
                }
                catch (...)
                {
                }
            }

            auto it = extracted.byFilename.find(pathStr);
            if (it != extracted.byFilename.end())
            {
                return it->second;
            }

            std::string pureName = std::filesystem::path(pathStr).filename().string();
            it                   = extracted.byFilename.find(pureName);
            if (it != extracted.byFilename.end())
            {
                return it->second;
            }

            return findTextureOnDisk(req.sourceFile, pathStr);
        };

        std::string           meshFileName = wstring_to_string(req.assetName) + ".mesh";
        std::filesystem::path meshPath     = assetFolder / meshFileName;

        struct BoneExportInfo
        {
            int         id;
            aiMatrix4x4 offsetMatrix;
        };
        std::unordered_map<std::string, BoneExportInfo> globalBoneMap;
        int                                             boneCounter = 0;

        {
            std::ofstream f(meshPath, std::ios::binary);
            uint32_t      n = scene->mNumMeshes;
            f.write((char*)&n, sizeof(uint32_t));

            for (uint32_t i = 0; i < n; ++i)
            {
                aiMesh*  m  = scene->mMeshes[i];
                uint32_t vc = m->mNumVertices, ic = m->mNumFaces * 3;
                f.write((char*)&vc, sizeof(uint32_t));
                f.write((char*)&ic, sizeof(uint32_t));

                std::vector<MeshVertex> vertices(vc);
                for (uint32_t v = 0; v < vc; ++v)
                {
                    vertices[v].px = m->mVertices[v].x;
                    vertices[v].py = m->mVertices[v].y;
                    vertices[v].pz = m->mVertices[v].z;

                    vertices[v].nx = m->mNormals ? m->mNormals[v].x : 0.0f;
                    vertices[v].ny = m->mNormals ? m->mNormals[v].y : 1.0f;
                    vertices[v].nz = m->mNormals ? m->mNormals[v].z : 0.0f;

                    vertices[v].tx = m->mTextureCoords[0] ? m->mTextureCoords[0][v].x : 0.0f;
                    vertices[v].ty = m->mTextureCoords[0] ? m->mTextureCoords[0][v].y : 0.0f;

                    for (int b = 0; b < 4; ++b)
                    {
                        vertices[v].boneIDs[b] = -1;
                        vertices[v].weights[b] = 0.0f;
                    }
                }

                if (m->HasBones())
                {
                    for (uint32_t boneIdx = 0; boneIdx < m->mNumBones; ++boneIdx)
                    {
                        aiBone*     bone     = m->mBones[boneIdx];
                        std::string boneName = bone->mName.C_Str();
                        int         boneID   = -1;

                        if (globalBoneMap.find(boneName) == globalBoneMap.end())
                        {
                            globalBoneMap[boneName] = {boneCounter, bone->mOffsetMatrix};
                            boneID                  = boneCounter;
                            boneCounter++;
                        }
                        else
                        {
                            boneID = globalBoneMap[boneName].id;
                        }

                        for (uint32_t wIdx = 0; wIdx < bone->mNumWeights; ++wIdx)
                        {
                            uint32_t vIdx   = bone->mWeights[wIdx].mVertexId;
                            float    weight = bone->mWeights[wIdx].mWeight;

                            for (int b = 0; b < 4; ++b)
                            {
                                if (vertices[vIdx].boneIDs[b] < 0)
                                {
                                    vertices[vIdx].boneIDs[b] = boneID;
                                    vertices[vIdx].weights[b] = weight;
                                    break;
                                }
                            }
                        }
                    }
                }

                f.write((char*)vertices.data(), sizeof(MeshVertex) * vc);

                for (uint32_t faceIdx = 0; faceIdx < m->mNumFaces; ++faceIdx)
                {
                    f.write((char*)m->mFaces[faceIdx].mIndices, sizeof(uint32_t) * 3);
                }
            }
        }
        result.assets.push_back({AssetType::MESH, meshPath});

        // Импорт материалов с использованием обновленного поиска текстур
        std::vector<std::string> matRelativePaths;
        for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
        {
            aiMaterial* aiMat = scene->mMaterials[i];
            aiString    name;
            aiMat->Get(AI_MATKEY_NAME, name);

            MaterialDesc d;
            d.name = name.length > 0 ? name.C_Str() : "mat_" + std::to_string(i);

            // Исправлено: теперь резервируем под все возможные каналы во избежание Out of Bounds
            d.textures.resize(6, "");

            aiString p;
            if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &p) == AI_SUCCESS)
            {
                d.textures[0] = resolveTexturePath(p.C_Str());
            }

            if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &p) == AI_SUCCESS ||
                aiMat->GetTexture(aiTextureType_HEIGHT, 0, &p) == AI_SUCCESS)
            {
                d.textures[1] = resolveTexturePath(p.C_Str());
            }

            // Исправлена проверка статуса (== AI_SUCCESS)
            if (aiMat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &p) == AI_SUCCESS ||
                aiMat->GetTexture(aiTextureType_LIGHTMAP, 0, &p) == AI_SUCCESS)
            {
                d.textures[2] = resolveTexturePath(p.C_Str());
            }

            if (aiMat->GetTexture(aiTextureType_METALNESS, 0, &p) == AI_SUCCESS)
            {
                d.textures[3] = resolveTexturePath(p.C_Str());
            }

            if (aiMat->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &p) == AI_SUCCESS)
            {
                d.textures[4] = resolveTexturePath(p.C_Str());
            }

            if (aiMat->GetTexture(aiTextureType_EMISSIVE, 0, &p) == AI_SUCCESS)
            {
                d.textures[5] = resolveTexturePath(p.C_Str());
            }

            auto mRes = importMaterial(d, assetFolder);
            if (mRes.success)
            {
                matRelativePaths.push_back(getAssetsRelativePath(mRes.assets.back().path));
                result.assets.insert(result.assets.end(), mRes.assets.begin(), mRes.assets.end());
            }
        }

        std::filesystem::path modelPath = assetFolder / (req.assetName + L".model");
        {
            std::ofstream f(modelPath);
            f << "{\n  \"mesh_source\": \"" << getAssetsRelativePath(meshPath) << "\",\n";

            f << "  \"bones\": [\n";
            int boneIndex = 0;
            for (auto const& [name, info] : globalBoneMap)
            {
                f << "    { \"name\": \"" << name << "\", \"id\": " << info.id
                  << ", \"offset\": " << mat4ToJson(info.offsetMatrix) << " }";
                if (boneIndex < globalBoneMap.size() - 1)
                {
                    f << ",";
                }
                f << "\n";
                boneIndex++;
            }
            f << "  ],\n";

            f << "  \"submeshes\": [\n";
            for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
            {
                uint32_t    mIdx     = scene->mMeshes[i]->mMaterialIndex;
                std::string mPathStr = (mIdx < matRelativePaths.size())
                                           ? matRelativePaths[mIdx]
                                           : "Assets/res/default.material";
                f << "    { \"id\": " << i << ", \"material\": \"" << mPathStr << "\" }";
                if (i < scene->mNumMeshes - 1)
                {
                    f << ",";
                }
                f << "\n";
            }
            f << "  ]\n}";
        }
        result.assets.push_back({AssetType::MODEL, modelPath});

        if (scene->mNumAnimations > 0)
        {
            for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
            {
                std::string animName = scene->mAnimations[i]->mName.length > 0
                                           ? scene->mAnimations[i]->mName.C_Str()
                                           : "anim_" + std::to_string(i);

                std::replace(animName.begin(), animName.end(), '|', '_');
                std::replace(animName.begin(), animName.end(), ':', '_');

                std::filesystem::path animAssetPath = assetFolder / (animName + ".anim");
                {
                    std::ofstream f(animAssetPath);
                    f << "{\n";
                    f << "  \"source\": \"" << getAssetsRelativePath(req.sourceFile) << "\",\n";
                    f << "  \"mesh\": \"" << getAssetsRelativePath(modelPath) << "\",\n";
                    f << "  \"track_index\": " << i << "\n";
                    f << "}";
                }
                result.assets.push_back({AssetType::ANIMATION, animAssetPath});
            }
        }

        return result;
    }

    std::filesystem::path AssetImporter::copyFile(
        const std::filesystem::path& src,
        const std::filesystem::path& dstDir
    ) noexcept
    {
        try
        {
            auto out = dstDir / src.filename();
            if (std::filesystem::exists(out) && std::filesystem::equivalent(src, out))
            {
                return out;
            }
            std::filesystem::copy_file(src, out, std::filesystem::copy_options::overwrite_existing);
            return out;
        }
        catch (...)
        {
            return {};
        }
    }

    bool AssetImporter::ensureDir(const std::filesystem::path& p) noexcept
    {
        try
        {
            if (!std::filesystem::exists(p))
            {
                return std::filesystem::create_directories(p);
            }
            return true;
        }
        catch (...)
        {
            return false;
        }
    }

    std::filesystem::path AssetImporter::toAssetPath(
        const std::filesystem::path& p,
        const std::string&           ext
    ) noexcept
    {
        auto c = p;
        c.replace_extension(ext);
        return c;
    }

} // namespace nb::SDK