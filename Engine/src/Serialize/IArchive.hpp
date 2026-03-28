#ifndef SRC_SERIALIZE_IARCHIVE_HPP
#define SRC_SERIALIZE_IARCHIVE_HPP

#include <filesystem>

namespace nb::Serialize
{
    class IArchive
    {
    public:

        enum class Mode
        {
            WRITE,
            READ
        };

        IArchive(const std::filesystem::path& pathToFile) noexcept;
        virtual ~IArchive() = default;

        virtual void value(
            const char* name,
            float& value
        ) noexcept = 0;

        virtual void value(
            const char* name,
            uint8_t& value
        ) noexcept = 0;

        virtual void value(
            const char* name,
            std::string& value
        ) noexcept = 0;

        virtual void value(
            const char* name,
            uint32_t& value
        ) noexcept = 0;

        virtual void value(
            const char* name,
            int32_t& value
        ) noexcept = 0;

         virtual void value(
            const char* name,
            bool& value
        ) noexcept = 0;

        virtual void beginArray(const char* name) noexcept = 0; 
        virtual void endArray() noexcept = 0;

        virtual void beginObject(const char* name) noexcept = 0;
        virtual void endObject() noexcept = 0;

        
        void setMode(Mode newMode) noexcept;

        bool isRead() const noexcept;
        bool isWrite() const noexcept;

        virtual void save() noexcept = 0;
        virtual bool load() noexcept = 0;


    protected:

        Mode mode;
        std::filesystem::path path;

    };
}

#endif