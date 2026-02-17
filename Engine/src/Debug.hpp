#pragma once

//#ifdef _DEBUG
#define NOMINMAX

#include <Windows.h>

#include <NbCore.hpp>

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <list>
#include <forward_list>
#include <deque>
#include <queue>
#include <vector>
#include <stack>
#include <array>

#include <type_traits>
#include <source_location>
#include <sstream>

#include "Math/Vector2.hpp"

class Debug
{

public:

    template <typename T>
    static void debug(T val
                    , const std::source_location& location = std::source_location::current())
    {
#if 0
        std::ostringstream oss;

        addHeader(location, oss);

        if constexpr (std::is_same_v<T, nb::Math::Vector2<float>>)
        {
            oss << val.x << " " << val.y << '\n';
        }
        else
        {
            oss << val << '\n';
        }

        writeToConsole(oss.str());
#endif
    }

    template <
        template <typename...> class Container,
        class... T
    >
    NB_DEPRECATED("Use ErrorManager::report instead")
    static void debug(
        const Container<T...> &container, 
        const std::source_location& location = std::source_location::current())
    {

#if 0
        std::ostringstream oss;
        addHeader(location, oss);

        if constexpr (is_map<Container<T...>>::value)
        {
            oss << "[ \n";
            for (const auto &i : container)
            {
                oss << "\t { " << i.first << ", " << i.second << " }\n";
            }
            oss << "] \n";
        }
        else if constexpr (is_set<Container<T...>>::value)
        {
            oss << "[ \n";
            for (const auto &i : container)
            {
                oss << "\t" << i << "\n";
            }
            oss << "] \n";
        }
        else if constexpr (std::is_same_v<Container<T...>, std::list<typename Container<T...>::value_type>> 
                            || std::is_same_v<Container<T...>, std::forward_list<typename Container<T...>::value_type>> 
                            || std::is_same_v<Container<T...>, std::vector<typename Container<T...>::value_type>> 
                            || std::is_same_v<Container<T...>, std::deque<typename Container<T...>::value_type>> 
                            || std::is_same_v<Container<T...>, std::array<typename Container<T...>::value_type, sizeof...(T)>>)
        {

            oss << " [\t";

            for (const auto &i : container)
            {
                oss << i << ", ";
            }

            // some shit that removes last ","
            if (!container.empty())
            {
                oss.seekp(-2, std::ios_base::end);
            }

            oss << "\t]\n";
        }
        else if constexpr (std::is_same_v<Container<T...>, std::string>
                            || std::is_same_v<Container<T...>, std::string_view>)
        {
            oss << container << '\n';
        }
        else if constexpr (std::is_same_v<Container<T...>, std::wstring>
                            || std::is_same_v<Container<T...>, std::wstring_view>)
        {
            std::wostringstream woss;
            addHeaderUnicode(location, woss);
            woss << container << '\n';
            writeToConsoleUnicode(woss.str());
        }
        else
        {
            oss << "Type not supported" << '\n';
        }

        writeToConsole(oss.str());
#endif
    } 

private:
    static void writeToConsole(std::string_view message)
    {
        HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdOut != NULL && stdOut != INVALID_HANDLE_VALUE)
        {
            DWORD written = 0;
            WriteConsoleA(stdOut, message.data(), static_cast<DWORD>(message.size()), &written, NULL);
        }
    }

    static void writeToConsoleUnicode(std::wstring_view message)
    {
        HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (stdOut != NULL && stdOut != INVALID_HANDLE_VALUE)
        {
            DWORD written = 0;
            WriteConsoleW(stdOut, message.data(), static_cast<DWORD>(message.size()), &written, NULL);
        }
    }

    static void addHeader(const std::source_location &location, std::ostringstream &oss)
    {
        oss << "In function: "
            << std::string(location.function_name())
            << " at line: "
            << std::to_string(location.line())
            << " ";
    }

     static void addHeaderUnicode(const std::source_location &location, std::wostringstream &oss)
    {
        oss << L"In function: "
            //<< std::wstring(location.function_name())
            << L" at line: "
            << std::to_wstring(location.line())
            << " ";
    }

    
    template <typename T>
    struct is_map : std::false_type
    {
    };

    template <typename... Args>
    struct is_map<std::map<Args...>> : std::true_type
    {
    };

    template <typename... Args>
    struct is_map<std::unordered_map<Args...>> : std::true_type
    {
    };

    template <typename T>
    struct is_set : std::false_type
    {
    };

    template <typename... Args>
    struct is_set<std::set<Args...>> : std::true_type
    {
    };

    template <typename... Args>
    struct is_set<std::unordered_set<Args...>> : std::true_type
    {
    };
};


template <>
inline void Debug::debug<std::wstring>(std::wstring val, const std::source_location& location)
{
    std::wostringstream woss;
    addHeaderUnicode(location, woss);
    woss << val << L"\n";
    writeToConsoleUnicode(woss.str());
}



//#endif // _DEBUG