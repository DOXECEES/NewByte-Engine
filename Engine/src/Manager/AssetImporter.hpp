#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include <Core.hpp>
#include <NonOwningPtr.hpp>
#include "ResourceManager.hpp"

namespace nb::SDK
{
    enum class AssetType
    {
        MESH,
        MODEL,
        TEXTURE,
        MATERIAL
    };

    struct ImportedAssetInfo
    {
        AssetType             type;
        std::filesystem::path path;
    };

    struct ImportSettings
    {
        bool importMeshes    = true;
        bool importMaterials = true;
        bool importTextures  = true;

        bool generateTangents = true;
        bool flipUV           = false;

        bool copyTexturesIntoProject = true;
    };

    struct ImportRequest
    {
        std::filesystem::path sourceFile;
        std::filesystem::path targetFolder;
        std::wstring          assetName;
        ImportSettings        settings;
    };

    struct ImportResult
    {
        bool                           success = true;
        std::wstring                   error;
        std::vector<ImportedAssetInfo> assets;
    };

    class AssetImporter
    {
    public:
        AssetImporter() = default;

        ImportResult import(const ImportRequest& req) noexcept;

    private:
        struct MaterialDesc
        {
            std::string                        name;
            std::vector<std::filesystem::path> textures;
        };

        ImportResult importModel(const ImportRequest& req) noexcept;
        ImportResult importTexture(
            const std::filesystem::path& src,
            const std::filesystem::path& dst
        ) noexcept;
        ImportResult importMaterial(
            const MaterialDesc&          mat,
            const std::filesystem::path& dstFolder
        ) noexcept;

        std::filesystem::path makeAssetFolder(const ImportRequest& req) noexcept;
        std::filesystem::path copyFile(
            const std::filesystem::path& src,
            const std::filesystem::path& dstFolder
        ) noexcept;

        std::filesystem::path toAssetPath(
            const std::filesystem::path& p,
            const std::string&           ext
        ) noexcept;

        bool ensureDir(const std::filesystem::path& p) noexcept;

    private:
    };
} // namespace nb::SDK