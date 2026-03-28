#pragma once

#include "IArchive.hpp"
#include "Loaders/JSON/Json.hpp"
#include <fstream>
#include <stack>
#include <string>
#include <vector>

namespace nb::Serialize
{
    class JsonArchive : public IArchive
    {
    public:
        JsonArchive(const std::filesystem::path& pathToFile) noexcept;

        void value(
            const char* name,
            float& value
        ) noexcept override;

         void value(
            const char* name,
            uint8_t& value
        ) noexcept override;

        void value(
            const char* name,
            std::string& value
        ) noexcept override;
        void value(
            const char* name,
            uint32_t& value
        ) noexcept override;
        void value(
            const char* name,
            int32_t& value
        ) noexcept override;
        void value(
            const char* name,
            bool& value
        ) noexcept override;

        void beginArray(const char* name) noexcept override;
        void endArray() noexcept override;

        void beginObject(const char* name) noexcept override;
        void endObject() noexcept override;

        void save() noexcept override;

        bool load() noexcept override;
        
        const Loaders::Json& getJson()
        {
            return json;
        }

    private:
        enum class ContextType
        {
            OBJECT,
            ARRAY
        };

        struct Context
        {
            ContextType type;
            bool firstElement;
        };

    private:
        std::string buffer;
        std::stack<Context> contextStack;
        Loaders::Json json;
    private:
        void writeIndent() noexcept;
        void writeCommaIfNeeded() noexcept;
        void writeNameIfNeeded(const char* name) noexcept;

        void writeString(const std::string& str) noexcept;
        void writeCString(const char* str) noexcept;

        void writeFloat(float v) noexcept;
        void writeInt(int32_t v) noexcept;
        void writeUInt(uint32_t v) noexcept;
        void writeBool(bool v) noexcept;

        void beginContainer(
            ContextType type,
            const char* name
        ) noexcept;
        void endContainer(ContextType type) noexcept;

        static std::string escapeString(const std::string& str) noexcept;
    };
} // namespace nb::Serialize