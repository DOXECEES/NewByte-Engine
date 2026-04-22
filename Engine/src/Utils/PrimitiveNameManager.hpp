#ifndef SRC_UTILS_PRIMITIVENAMEMANAGER_HPP
#define SRC_UTILS_PRIMITIVENAMEMANAGER_HPP
#include <charconv>
#include <format>
#include <queue>
#include <string>
#include <string_view>
#include <unordered_map>

#include "Error/ErrorManager.hpp"

namespace nb::Utils
{

    class PrimitiveNameManager
    {
    public:
        PrimitiveNameManager()  = default;
        ~PrimitiveNameManager() = default;

        std::string generateName(std::string_view typeName) noexcept;

        void releaseName(std::string_view fullName) noexcept;

    private:
        struct IndexPool
        {
            int             globalIndex = 1;
            std::queue<int> releasedIndexes;
        };

        std::string cleanTypeName(std::string_view typeName) const noexcept;

        std::unordered_map<std::string, IndexPool> pools;
    };
} // namespace nb::Utils
#endif // SRC_UTILS_PRIMITIVENAMEMANAGER_HPP