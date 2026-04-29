#include "AssetImporter.hpp"
#include <Debug.hpp>
#include <algorithm>
#include <fstream>
#include <string>
#include <vector>

// Assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace nb::SDK
{

    struct MeshVertex
    {
        float px, py, pz;
        float nx, ny, nz;
        float tx, ty;
    };

    // Вспомогательная функция для получения пути формата "Assets/..."
    static std::string getAssetsRelativePath(const std::filesystem::path& fullPath)
    {
        std::string pathStr = fullPath.generic_string(); // Использует '/' даже на Windows
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

    // ---------------- TEXTURE IMPORT ----------------
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
              << "\",\n  \"filter\": \"linear\"\n}";
        }
        result.success = true;
        result.assets.push_back({AssetType::TEXTURE, texAsset});
        return result;
    }

    // ---------------- MATERIAL IMPORT ----------------
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

            auto writeTexProp =
                [&](const std::string& propName, const std::filesystem::path& srcPath, bool isLast)
            {
                std::string assetPath = "Assets/res/placeholder.texture";
                if (!srcPath.empty())
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

            writeTexProp("u_AlbedoMap", mat.textures.size() > 0 ? mat.textures[0] : "", false);
            writeTexProp("u_NormalMap", mat.textures.size() > 1 ? mat.textures[1] : "", false);
            writeTexProp("u_ORMMap", mat.textures.size() > 2 ? mat.textures[2] : "", false);

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

    // ---------------- MODEL IMPORT ----------------
    ImportResult AssetImporter::importModel(const ImportRequest& req) noexcept
    {
        ImportResult result;
        result.success                    = true;
        std::filesystem::path assetFolder = req.targetFolder / req.assetName;
        ensureDir(assetFolder);

        Assimp::Importer importer;
        const aiScene*   scene = importer.ReadFile(
            req.sourceFile.string(), aiProcess_Triangulate | aiProcess_CalcTangentSpace
        );
        if (!scene)
        {
            return {false, L"Assimp Load Error"};
        }

        // 1. .mesh (Binary)
        std::string           meshFileName = wstring_to_string(req.assetName) + ".mesh";
        std::filesystem::path meshPath     = assetFolder / meshFileName;
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
                for (uint32_t v = 0; v < vc; ++v)
                {
                    MeshVertex vert;
                    vert.px = m->mVertices[v].x;
                    vert.py = m->mVertices[v].y;
                    vert.pz = m->mVertices[v].z;
                    vert.nx = m->mNormals[v].x;
                    vert.ny = m->mNormals[v].y;
                    vert.nz = m->mNormals[v].z;
                    vert.tx = m->mTextureCoords[0] ? m->mTextureCoords[0][v].x : 0.0f;
                    vert.ty = m->mTextureCoords[0] ? m->mTextureCoords[0][v].y : 0.0f;
                    f.write((char*)&vert, sizeof(MeshVertex));
                }
                for (uint32_t faceIdx = 0; faceIdx < m->mNumFaces; ++faceIdx)
                {
                    f.write((char*)m->mFaces[faceIdx].mIndices, sizeof(uint32_t) * 3);
                }
            }
        }
        result.assets.push_back({AssetType::MESH, meshPath});

        // 2. Материалы
        std::vector<std::string> matRelativePaths;
        for (uint32_t i = 0; i < scene->mNumMaterials; ++i)
        {
            aiMaterial* aiMat = scene->mMaterials[i];
            aiString    name;
            aiMat->Get(AI_MATKEY_NAME, name);
            MaterialDesc d;
            d.name = name.length > 0 ? name.C_Str() : "mat_" + std::to_string(i);

            aiString p;
            if (aiMat->GetTexture(aiTextureType_DIFFUSE, 0, &p) == AI_SUCCESS)
            {
                d.textures.push_back(findTextureOnDisk(req.sourceFile, p.C_Str()));
            }
            if (aiMat->GetTexture(aiTextureType_NORMALS, 0, &p) == AI_SUCCESS)
            {
                d.textures.push_back(findTextureOnDisk(req.sourceFile, p.C_Str()));
            }

            auto mRes = importMaterial(d, assetFolder);
            if (mRes.success)
            {
                matRelativePaths.push_back(getAssetsRelativePath(mRes.assets.back().path));
                result.assets.insert(result.assets.end(), mRes.assets.begin(), mRes.assets.end());
            }
        }

        // 3. .model
        std::filesystem::path modelPath = assetFolder / (req.assetName + L".model");
        {
            std::ofstream f(modelPath);
            f << "{\n  \"mesh_source\": \"" << getAssetsRelativePath(meshPath)
              << "\",\n  \"submeshes\": [\n";
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