#ifndef SRC_LOADERS_JSON_JSONPARSER_HPP
#define SRC_LOADERS_JSON_JSONPARSER_HPP

#include <string> 
#include <vector>
#include <map>
#include <sstream>

#include "Node.hpp"
#include "JsonLexer.hpp"

namespace nb
{
    namespace Loaders
    {
        /**
         * @class JsonParser
         * @brief Класс для парсинга JSON строк и файлов.
         * @todo Класс исключений для Parser
         * @todo Унаследовать от ILoader
         */
        class JsonParser
        {
        public:
            JsonParser() = default;

            /**
             * @brief Конструктор для парсинга JSON файла по пути.
             * @param path Путь к JSON файлу.
             */
            inline explicit JsonParser(const std::filesystem::path &path) 
                : lexer(path)
                , token(getTokenNoWhiteSpaces())
            {}

            /**
             * @brief Конструктор для парсинга JSON строки.
             * @param json Строка JSON.
             */
            inline explicit JsonParser(const std::string& json) 
                : lexer(json)
                , token(getTokenNoWhiteSpaces())
            {}


            inline void setPath(const std::filesystem::path &path)
            {
                lexer.setPath(path);
                token = getTokenNoWhiteSpaces();
            }

            inline void setJson(const std::string& json)
            {
                lexer.setJson(json);
                token = getTokenNoWhiteSpaces();
            }

            /**
             * @brief Основной метод для парсинга JSON.
             * @return Корневой элемент JSON как Node.
             */
            Node parse();

        private:

             /**
             * @brief Разбирает строковое значение токена.
             * @param curToken Текущий токен.
             * @return Строковое значение токена.
             */
            inline std::string parseString(const JsonLexer::Token &curToken)
            {
                return curToken.value;
            }

            /**
             * @brief Разбирает JSON объект.
             * @return Объект JSON как Node::Map.
             */
            Node::Map parseObj();

            /**
             * @brief Разбирает ключ-значение пары в объекте.
             * @return Значение как Node.
             */
            Node parsePair();

            /**
             * @brief Разбирает значение JSON.
             * @return Значение как Node.
             */
            Node parseValue();

            /**
             * @brief Разбирает ключ в объекте JSON.
             * @return Ключ как строку.
             */
            std::string parseKey();

            /**
             * @brief Разбирает массив JSON.
             * @return Массив как Node::Array.
             */
            Node::Array parseArray();

            /**
             * @brief Разбирает логические выражения (true, false, null).
             * @return Логическое выражение как Node.
             */
            Node parseLogicExpr();

            /**
             * @brief Получает следующий токен, игнорируя пробелы.
             * @return Следующий токен JSON.
             */
            JsonLexer::Token getTokenNoWhiteSpaces();

            /**
             * @brief Разбирает числовые значения.
             * @return Числовое значение как Node.
             */
            Node parseNumber();

        private:

            JsonLexer lexer;
            JsonLexer::Token token;
        
        };
    };
};

#endif
