#include "Node.hpp"

namespace nb
{
    namespace Loaders
    {
        Node &Node::operator[](const std::string &key)
        {
            if (isObject())
            {
                return std::get<Map>(data)[key];
            }
            else
            {
                throw std::runtime_error("This node isnt object");
            }
        }

        auto Node::operator[](const size_t index)
        {
            if (isArray())
            {
                return std::get<Array>(data)[index];
            }
            else
            {
                throw std::runtime_error("This isnt array");
            }
        }

    };
};
