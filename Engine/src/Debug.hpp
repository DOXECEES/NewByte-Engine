#pragma once

#ifdef _DEBUG

#include <Windows.h>

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
                    , const std::source_location location = std::source_location::current())
    {

        std::ostringstream oss;

        addHeader(location, oss);

        if constexpr (std::is_same_v<T, nb::Math::Vector2>)
        {
            oss << val.x << " " << val.y << '\n';
        }
        else
        {
            oss << val << '\n';
        }

        writeToConsole(oss.str());
    }

    template <template <typename...> class Container, class... T>
    static void debug(const Container<T...> &container
                    , const std::source_location location = std::source_location::current())
    {

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
        else
        {
            oss << "Type not supported" << '\n';
        }

        writeToConsole(oss.str());
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

    static void addHeader(const std::source_location &location, std::ostringstream &oss)
    {
        oss << "In function: "
            << std::string(location.function_name())
            << " at line: "
            << std::to_string(location.line())
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

#endif // _DEBUG