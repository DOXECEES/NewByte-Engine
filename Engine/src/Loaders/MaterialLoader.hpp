#ifndef SRC_LOADERS_MATERIALLOADER_HPP
#define SRC_LOADERS_MATERIALLOADER_HPP

#include "Resources/MaterialAsset.hpp"

#include <filesystem>

namespace nb::Loaders
{
    class MaterialLoader
    {
    public:
        MaterialLoader() = default;
        ~MaterialLoader() = default;

        Ref<Resource::MaterialAsset> loadAsset(const std::filesystem::path& path) noexcept;

    private:
    };
}; // namespace nb::Loaders

#endif