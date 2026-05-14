#include "StringUtils.hpp"

#include <Windows.h>

namespace nb::Utils
{
    std::wstring toWString(const std::string& str) noexcept
    {
        int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);

        std::wstring wstrTo(sizeNeeded, 0);
        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], sizeNeeded);

        return wstrTo;
    }

    void trimLast(
        std::string& str,
        char         trimChar
    ) noexcept
    {
        auto pos = str.find_last_not_of(trimChar);
        str.erase(pos == std::string::npos ? 0 : pos + 1);
    }

    


};
