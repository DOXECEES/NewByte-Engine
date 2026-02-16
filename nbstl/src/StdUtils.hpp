#ifndef NBSTL_SRC_STDUTILS_HPP
#define NBSTL_SRC_STDUTILS_HPP

#include <Windows.h>

namespace nbstl 
{
    inline std::string trim(const std::string& s)
    {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
        return s.substr(start, end - start);
    }

    inline std::string_view trim(std::string_view s) noexcept
    {
        size_t start = 0;
        while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;

        size_t end = s.size();
        while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;

        return s.substr(start, end - start);
    }

    inline std::wstring toWString(std::string_view str) noexcept
    {
        if (str.empty())
        {
            return L"";
        }
        int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), nullptr, 0);
        std::wstring utf16(len, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, str.data(), (int)str.size(), utf16.data(), len);
        return utf16;
    }


}; 

#endif 
