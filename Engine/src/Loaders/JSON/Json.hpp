#ifndef SRC_LOADERS_JSON_JSON_HPP
#define SRC_LOADERS_JSON_JSON_HPP

#include <filesystem>

#include "Node.hpp"
#include "JsonParser.hpp"
#include "JsonWriter.hpp"

namespace nb
{
    namespace Loaders
    {
        /**
        * @brief Класс для работы с данными в формате json
        */

        class Json
        {
        public:

            Json() = default;
            ~Json() = default;
            Json(const Json&) = default;
            Json& operator=(const Json&) = default;
            Json(Json&&) noexcept = default;
            Json& operator=(Json&&) noexcept = default;

            inline explicit Json(const std::filesystem::path& path)
                : parser(path)
                , writer()
            {
                parse();
            }

            inline explicit Json(const std::string &json)
                : parser(json)
                , writer()
            {
                parse();
            }

            inline void write()
            {
                writer.write(root);
            }

            inline void parse()
            {
                root = parser.parse();
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
    };
};

#endif
