#ifndef SRC_LOADERS_JSON_JSON_HPP
#define SRC_LOADERS_JSON_JSON_HPP

#include <filesystem>

#include "../../Resources/IResource.hpp"
#include "../Factory/IFactoryLoader.hpp"

#include "../../Core.hpp"

#include "Node.hpp"
#include "JsonParser.hpp"
#include "JsonWriter.hpp"

namespace nb
{
    namespace Loaders
    {
        class Json;

        


        class Json : public nb::Resource::IResource, public IReadableWriteable
        {
        public:

            Json() = default;
            ~Json() = default;
            Json(const Json&) = default;
            Json& operator=(const Json&) = default;
            Json(Json&&) noexcept = default;
            Json& operator=(Json&&) noexcept = default;

            inline explicit Json(const std::filesystem::path& path)
                : parser()
                , writer()
            {
                readFromFile(path);
            }

            inline explicit Json(const std::string &json)
                : parser()
                , writer()
            {
                readFromMemory(json);
            }

            void readFromFile(const std::filesystem::path& path) override
            {
                parser.setPath(path);
                root = parser.parse();
            }

            void writeToFile(const std::filesystem::path& path) override
            {
                //parser.setJson()
                writer.write(root);
            }

            void readFromMemory(const std::string &json)
            {
                parser.setJson(json);
                root = parser.parse();
            }

            inline void write()
            {
                writer.write(root);
            }

            template<typename T>
            auto operator[](const T& index) const
            {
                return root[index];
            }

            template<typename T>
            auto operator[](const T& index)
            {
                return root[index];  
            }

        private:
            Node root;

            JsonParser parser;
            JsonWriter writer;
        };


        /**
        * @brief Класс для работы с данными в формате json
        */
        class JsonFactory : public Factory::IFactoryLoader
        {
        public:
            JsonFactory() = default;
            Ref<Resource::IResource> create(const std::filesystem::path& path) const override;
            std::type_index getResourceType() const noexcept override { return std::type_index(typeid(nb::Loaders::Json)); }

        private:


        };

    };
};

#endif
