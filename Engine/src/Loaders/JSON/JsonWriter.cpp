// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "JsonWriter.hpp"

namespace nb
{
    namespace Loaders
    {
        void JsonWriter::travel(const Node &root)
        {
            std::stack<std::string> keys;
            std::stack<Node> stack;

            stack.push(root);
            keys.push("");

            while (!stack.empty())
            {
                Node current = stack.top();
                std::string currentKey = keys.top();

                stack.pop();
                keys.pop();

                if (current.isObject())
                {
                    writeObject(currentKey);

                    Node::Map map = current.get<Node::Map>();
                    for (auto i = map.begin(); i != map.end(); ++i)
                    {
                        stack.push(i->second);
                        keys.push(i->first);
                    }
                }
                else if (current.isValue())
                {
                    if (current.isString())
                    {
                        writeString(currentKey, current.stringify());
                    }
                    else
                    {
                        writeValue(currentKey, current.stringify());
                    }
                }
                else
                {
                    writeArray(currentKey, current.get<Node::Array>());
                }

                if (symbols.top() == ',' && !keys.empty())
                {
                    output << symbols.top() << '\n';
                    symbols.pop();
                }
            }

            output << '\n';

            while (!symbols.empty())
            {
                if (symbols.top() != ',')
                {
                    repeatTabs(symbols.size() - 1);
                    output << symbols.top() << '\n';
                }
                symbols.pop();
            }
        }

        void JsonWriter::writeObject(std::string_view key)
        {
            repeatTabs(symbols.size());

            if (!key.empty())
            {
                output << '"' << key << '"' << ": {\n";
            }
            else
            {
                output << "{\n";
            }
            symbols.push('}');
        }

        void JsonWriter::writeArray(std::string_view key, const Node::Array &arr)
        {
            repeatTabs(symbols.size());
            output << '"' << key << '"' << ": [\n";

            for (auto it = arr.begin(); it != arr.end(); ++it)
            {
                repeatTabs(symbols.size() + 1);

                if (it->isObject())
                {

                    std::stack<std::string> keys;
                    std::stack<Node> stack;

                    stack.push(*it);
                    keys.push("");

                    while (!stack.empty())
                    {
                        Node current = stack.top();
                        std::string currentKey = keys.top();

                        stack.pop();
                        keys.pop();

                        if (current.isObject())
                        {
                            writeObject(currentKey);

                            Node::Map map = current.get<Node::Map>();
                            for (auto i = map.begin(); i != map.end(); ++i)
                            {
                                stack.push(i->second);
                                keys.push(i->first);
                            }
                        }
                        else if (current.isValue())
                        {
                            if (current.isString())
                            {
                                writeString(currentKey, current.stringify());
                            }
                            else
                            {
                                writeValue(currentKey, current.stringify());
                            }
                        }
                        else
                        {
                            writeArray(currentKey, current.get<Node::Array>());
                        }

                        if (symbols.top() == ',' && !keys.empty())
                        {
                            output << symbols.top() << '\n';
                            symbols.pop();
                        }
                    }

                    output << '\n';

                    while (!symbols.empty())
                    {
                        if (symbols.top() != ',')
                        {
                            repeatTabs(symbols.size() - 1);
                            output << symbols.top() << '\n';
                        }
                        symbols.pop();
                    }
                }
                else
                {
                    if (it->isString())
                    {
                        output << '"' << it->stringify() << '"';
                    }
                    else
                    {
                        output << it->stringify();
                    }

                    if (std::next(it) != arr.end())
                    {
                        output << ',';
                    }

                    output << '\n';
                }
            }

            repeatTabs(symbols.size());
            output << ']';
            symbols.push(',');
        }

        void JsonWriter::writeValue(std::string_view key, std::string_view value) noexcept
        {
            repeatTabs(symbols.size());
            output << '"' << key << '"' << ": " << value;
            symbols.push(',');
        }

        void JsonWriter::writeString(std::string_view key, std::string_view value) noexcept
        {
            repeatTabs(symbols.size());
            output << '"' << key << '"' << ": " << "\"" << value << "\"";
            symbols.push(',');
        }

        void JsonWriter::repeatTabs(size_t count) noexcept
        {
            for (size_t i = 0; i < count; i++)
            {
                output << '\t';
            }
        }
    };
};