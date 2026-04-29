#include "PrimitiveNameManager.hpp"

namespace nb::Utils
{

    std::string PrimitiveNameManager::generateName(std::string_view typeName) noexcept
    {
        std::string baseName = cleanTypeName(typeName);
        auto&       pool     = pools[baseName];

        int index = 0;
        if (!pool.releasedIndexes.empty())
        {
            index = pool.releasedIndexes.front();
            pool.releasedIndexes.pop();

            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::INFO, "Reusing released index for primitive")
                .with("baseName", baseName)
                .with("index", index);
        }
        else
        {
            index = pool.globalIndex++;
        }

        return std::format("{} {}", baseName, index);
    }
    void PrimitiveNameManager::releaseName(std::string_view fullName) noexcept
    {
        const size_t lastSpace = fullName.find_last_of(' ');
        if (lastSpace == std::string_view::npos)
        {
            nb::Error::ErrorManager::instance()
                .report(
                    nb::Error::Type::WARNING, "Failed to release name: Invalid format (no space found)"
                )
                .with("fullName", std::string(fullName));
            return;
        }

        std::string_view baseNameView = fullName.substr(0, lastSpace);
        std::string_view indexPart    = fullName.substr(lastSpace + 1);

        int index = 0;
        auto [ptr, error] =
            std::from_chars(indexPart.data(), indexPart.data() + indexPart.size(), index);

        if (error == std::errc())
        {
            pools[std::string(baseNameView)].releasedIndexes.push(index);
        }
        else
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::WARNING, "Failed to parse index for release")
                .with("fullName", std::string(fullName))
                .with("indexPart", std::string(indexPart));
        }
    }
    std::string PrimitiveNameManager::cleanTypeName(std::string_view typeName) const noexcept
    {
        static constexpr std::string_view PARAMS_SUFFIX = "Params";

        std::string  name(typeName);
        const size_t pos = name.find(PARAMS_SUFFIX);

        if (pos != std::string::npos)
        {
            return name.substr(0, pos);
        }

        return name;
    }
}; // namespace nb::Utils