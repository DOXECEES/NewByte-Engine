#include "JsonArchive.hpp"

#include <cstdio>
#include <cstring>

namespace nb::Serialize
{
    JsonArchive::JsonArchive(const std::filesystem::path& pathToFile) noexcept
        : IArchive(pathToFile)
    {
        buffer.reserve(4096);
    }

    void JsonArchive::writeIndent() noexcept
    {
        buffer.push_back('\n');

        const size_t depth = contextStack.size();
        for (size_t i = 0; i < depth; ++i)
        {
            buffer.append("    ");
        }
    }

    void JsonArchive::writeCommaIfNeeded() noexcept
    {
        if (!contextStack.empty())
        {
            Context& ctx = contextStack.top();
            if (!ctx.firstElement)
            {
                buffer.push_back(',');
            }
            else
            {
                ctx.firstElement = false;
            }
        }
    }

    void JsonArchive::writeNameIfNeeded(const char* name) noexcept
    {
        if (contextStack.empty())
        {
            return;
        }

        Context& ctx = contextStack.top();

        if (ctx.type == ContextType::OBJECT)
        {
            writeIndent();

            if (name)
            {
                writeCString(name);
            }
            else
            {
                writeCString("");
            }

            buffer.append(": ");
        }
        else
        {
            writeIndent();
        }
    }

    void JsonArchive::writeCString(const char* str) noexcept
    {
        buffer.push_back('"');

        if (str)
        {
            while (*str)
            {
                const char c = *str;

                switch (c)
                {
                case '\\':
                    buffer.append("\\\\");
                    break;
                case '"':
                    buffer.append("\\\"");
                    break;
                case '\n':
                    buffer.append("\\n");
                    break;
                case '\r':
                    buffer.append("\\r");
                    break;
                case '\t':
                    buffer.append("\\t");
                    break;
                default:
                    buffer.push_back(c);
                    break;
                }

                ++str;
            }
        }

        buffer.push_back('"');
    }

    std::string JsonArchive::escapeString(const std::string& str) noexcept
    {
        std::string out;
        out.reserve(str.size() + 8);

        for (char c : str)
        {
            switch (c)
            {
            case '\\':
                out += "\\\\";
                break;
            case '"':
                out += "\\\"";
                break;
            case '\n':
                out += "\\n";
                break;
            case '\r':
                out += "\\r";
                break;
            case '\t':
                out += "\\t";
                break;
            default:
                out.push_back(c);
                break;
            }
        }

        return out;
    }

    void JsonArchive::writeString(const std::string& str) noexcept
    {
        buffer.push_back('"');
        buffer += escapeString(str);
        buffer.push_back('"');
    }

    void JsonArchive::writeFloat(float v) noexcept
    {
        char tmp[64];
        std::snprintf(tmp, sizeof(tmp), "%.6f", v);

        // убираем хвостовые нули и точку
        char* end = tmp + std::strlen(tmp) - 1;
        //while (end > tmp && *end == '0')
        //{
        //    *end = 0;
        //    --end;
        //}
        //if (end > tmp && *end == '.')
        //{
        //    *end = 0;
        //}

        buffer += tmp;
    }

    void JsonArchive::writeInt(int32_t v) noexcept
    {
        char tmp[32];
        std::snprintf(tmp, sizeof(tmp), "%d", v);
        buffer += tmp;
    }

    void JsonArchive::writeUInt(uint32_t v) noexcept
    {
        char tmp[32];
        std::snprintf(tmp, sizeof(tmp), "%u", v);
        buffer += tmp;
    }

    void JsonArchive::writeBool(bool v) noexcept
    {
        buffer += (v ? "true" : "false");
    }

    void JsonArchive::beginContainer(
        ContextType type,
        const char* name
    ) noexcept
    {
        if (!contextStack.empty())
        {
            writeCommaIfNeeded();
        }

        if (!contextStack.empty())
        {
            writeNameIfNeeded(name);
        }

        if (contextStack.empty())
        {
            // root container
        }

        if (type == ContextType::OBJECT)
        {
            buffer.push_back('{');
        }
        else
        {
            buffer.push_back('[');
        }

        contextStack.push({type, true});
    }

    void JsonArchive::endContainer(ContextType type) noexcept
    {
        if (contextStack.empty())
        {
            return;
        }

        if (contextStack.top().type != type)
        {
            return;
        }

        // если были элементы, красиво закрываем с переносом строки
        if (!contextStack.top().firstElement)
        {
            contextStack.pop();
            writeIndent();

            if (type == ContextType::OBJECT)
            {
                buffer.push_back('}');
            }
            else
            {
                buffer.push_back(']');
            }
        }
        else
        {
            contextStack.pop();

            if (type == ContextType::OBJECT)
            {
                buffer.push_back('}');
            }
            else
            {
                buffer.push_back(']');
            }
        }
    }

    void JsonArchive::beginObject(const char* name) noexcept
    {
        if (!isWrite())
        {
            return;
        }

        beginContainer(ContextType::OBJECT, name);
    }

    void JsonArchive::endObject() noexcept
    {
        if (!isWrite())
        {
            return;
        }

        endContainer(ContextType::OBJECT);
    }

    void JsonArchive::beginArray(const char* name) noexcept
    {
        if (!isWrite())
        {
            return;
        }

        beginContainer(ContextType::ARRAY, name);
    }

    void JsonArchive::endArray() noexcept
    {
        if (!isWrite())
        {
            return;
        }

        endContainer(ContextType::ARRAY);
    }

    void JsonArchive::value(
        const char* name,
        float& value
    ) noexcept
    {
        if (!isWrite())
        {
            return;
        }

        writeCommaIfNeeded();
        writeNameIfNeeded(name);
        writeFloat(value);
    }

    void JsonArchive::value(
        const char* name,
        uint8_t& value
    ) noexcept
    {
        if (!isWrite())
        {
            return;
        }

        writeCommaIfNeeded();
        writeNameIfNeeded(name);
        writeUInt(value);
    }

    void JsonArchive::value(
        const char* name,
        std::string& value
    ) noexcept
    {
        if (!isWrite())
        {
            return;
        }

        writeCommaIfNeeded();
        writeNameIfNeeded(name);
        writeString(value);
    }

    void JsonArchive::value(
        const char* name,
        uint32_t& value
    ) noexcept
    {
        if (!isWrite())
        {
            return;
        }

        writeCommaIfNeeded();
        writeNameIfNeeded(name);
        writeUInt(value);
    }

    void JsonArchive::value(
        const char* name,
        int32_t& value
    ) noexcept
    {
        if (!isWrite())
        {
            return;
        }

        writeCommaIfNeeded();
        writeNameIfNeeded(name);
        writeInt(value);
    }

    void JsonArchive::value(
        const char* name,
        bool& value
    ) noexcept
    {
        if (!isWrite())
        {
            return;
        }

        writeCommaIfNeeded();
        writeNameIfNeeded(name);
        writeBool(value);
    }

    void JsonArchive::save() noexcept
    {
        if (!isWrite())
        {
            return;
        }

        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
        {
            return;
        }

        file.write(buffer.data(), buffer.size());
    }

    bool JsonArchive::load() noexcept
    {
        if (!isRead())
        {
            return false;
        }

        json.readFromFile(path);

        return true;
    }
} // namespace nb::Serialize