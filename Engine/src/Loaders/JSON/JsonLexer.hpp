#ifndef SRC_LOADERS_JSON_JSONLEXER_HPP
#define SRC_LOADERS_JSON_JSONLEXER_HPP

#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace nb
{
    namespace Loaders
    {
         /**
         * @brief Класс для лексического анализа JSON-документов.
         * 
         * Этот класс читает JSON-файл и разбивает его на токены,
         * которые могут быть использованы для дальнейшего парсинга.
         */
        class JsonLexer
        {

        public:
            JsonLexer() = default;

            /**
             * @brief Конструктор, принимающий путь к JSON-файлу.
             * 
             * @param path Путь к файлу.
             */
            explicit JsonLexer(const std::filesystem::path &path);
            
             /**
             * @brief Конструктор, принимающий JSON-строку напрямую.
             * 
             * @param json Строка JSON.
             */
            explicit JsonLexer(const std::string &json) noexcept;
            
            JsonLexer(const JsonLexer &other) = delete;
            JsonLexer(JsonLexer &&other) = delete;
            JsonLexer &operator=(const JsonLexer &other) = delete;
            JsonLexer &operator=(JsonLexer &&other) noexcept = delete;

            ~JsonLexer() noexcept;

            /**
             * @brief Перечисление, представляющее типы токенов, которые могут быть распознаны.
             */
            enum class TokenType
            {
                OBJECT_BEGIN,     ///< '{'
                OBJECT_END,       ///< '}'
                STRING_SEPARATOR, ///< '"'
                NAME_SEPARATOR,   ///< ':'
                PAIR_SEPARATOR,   ///< ','
                ARRAY_BEGIN,      ///< '['
                ARRAY_END,        ///< ']'
                WHITESPACE,       ///< пробелы
                STRING,           ///< строковые значения
                NUMBER,           ///< числовые значения
                JTRUE,             ///< true
                JFALSE,            ///< false
                NILL,             ///< null
                NONE              ///< неверный токен
            };

            /**
             * @brief Структура, представляющая токен, содержащий тип и значение.
             */
            struct Token
            {
                TokenType type;
                std::string value;
            };

            /**
             * @brief Получает следующий токен из входных данных.
             * 
             * @return Токен с типом и значением.
             */
            Token nextToken();

            void setPath(const std::filesystem::path &path);
            void setJson(const std::string &json);

        private:
            inline bool isInRange() const noexcept
            {
                return (pos < size);
            }

            inline Token createToken(TokenType type, const std::string& value = "") noexcept
            {
                pos++;
                return {type, value};
            }

            Token parseOther() noexcept;

            Token parseLogicExpr() noexcept;

            Token parseNumber() noexcept;

            Token parseString() noexcept;

            std::string data = "";
            size_t size = 0;
            size_t pos = 0;
            std::ifstream file;
        };

    };
};

#endif
