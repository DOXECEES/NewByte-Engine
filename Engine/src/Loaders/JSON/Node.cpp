// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Node.hpp"

namespace nb
{
    namespace Loaders
    {
        bool Node::contains(const std::string& key) const noexcept
        {
            if (isObject())
            {
                return std::get<Map>(data).contains(key);
            }
            return false;
        }

        Node& Node::operator[](const std::string& key)
        {
            if (!isObject())
            {
                data = Map{};
            }

            return std::get<Map>(data)[key];
        }

        const Node& Node::operator[](const std::string& key) const
        {
            if (!isObject())
            {
                throw std::runtime_error("This node isnt object");
            }

            return std::get<Map>(data).at(key);
        }

        Node& Node::operator[](size_t index)
        {
            if (!isArray())
            {
                data = Array{};
            }

            auto& arr = std::get<Array>(data);

            if (index >= arr.size())
            {
                arr.resize(index + 1);
            }

            return arr[index];
        }

        const Node& Node::operator[](size_t index) const
        {
            if (!isArray())
            {
                throw std::runtime_error("This node isnt array");
            }

            return std::get<Array>(data).at(index);
        }

    };
};
