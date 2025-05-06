#ifndef SRC_LOADERS_JSON_JSONWRITER_HPP
#define SRC_LOADERS_JSON_JSONWRITER_HPP

#include <map>
#include <stack>
#include <string> 
#include <sstream>

#include "Node.hpp"

namespace nb
{
    namespace Loaders
    {

        /**
         * @brief Класс для записи JSON структуры из Json-дерева в строку.
         * @todo  Добавить поддержку массивов объектов.
         * @todo  Запретить запись элементов разных типов в массив
         */
        class JsonWriter
        {
        public:
            JsonWriter() = default;
            ~JsonWriter() = default;

             /**
             * @brief Записывает объект Node в JSON-формат
             * @param node Корневой узел
             */
            inline void write(Node &node)
            {
                output.clear();
                return travel(node);
            }

        private:
            void travel(const Node &root);

            /**
             * @brief Запись объекта JSON
             * @param key Ключ объекта
             */
            void writeObject(std::string_view key);

            /**
             * @brief Запись массива JSON
             * @param key Ключ массива
             * @param arr Массив для записи
             */
            void writeArray(std::string_view key, const Node::Array &arr);

            /**
             * @brief Запись значения в формате JSON
             * @param key Ключ значения
             * @param value Значение
             */
            void writeValue(std::string_view key, std::string_view value) noexcept;
            
            /**
             * @brief Запись строки в формате JSON
             * @param key Ключ строки
             * @param value Строковое значение
             */
            void writeString(std::string_view key, std::string_view value) noexcept;

            /**
             * @brief Добавление табуляций для форматирования
             * @param count Количество табуляций
             */
            void repeatTabs(size_t count) noexcept;


        private:
            
            std::ostringstream  output;
            std::stack<char>    symbols;
        };

    };
};

#endif
