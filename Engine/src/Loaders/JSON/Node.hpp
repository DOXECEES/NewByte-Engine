#ifndef SRC_LOADERS_JSON_NODE_HPP
#define SRC_LOADERS_JSON_NODE_HPP

#include <map>
#include <string> 
#include <vector>
#include <variant>
#include <sstream>
#include <filesystem>

namespace nb
{
    namespace Loaders
    {
        /**
         * @brief Класс представляет собой узел дерева элементов json
         * @todo класс исключений для обработки ошибок
         */ 
        class Node
        {
        public:

            using Map = std::map<std::string, Node>;
            using Value = std::variant<int, float, bool, std::string, std::nullptr_t>;
            using Array = std::vector<Node>;

            inline Node() = default;
            inline Node(const Node &other) = default;
            inline Node(Node &&other) = default;
            inline Node& operator=(const Node &other) = default;
            Node& operator=(Node&& other) noexcept = default;


            inline Node(std::nullptr_t value) noexcept : data(value) {}
            inline Node(bool value) noexcept : data(value) {}
            inline Node(int value) noexcept : data(value) {}
            inline Node(float value) noexcept : data(value) {}
            inline Node(const std::string &value) noexcept : data(value) {}
            inline Node(const char *value) noexcept : data(std::string(value)) {}

            inline Node(const Map &map) noexcept : data(map) {}
            inline Node(const Array &arr) noexcept: data(arr) {}

            inline Node(std::initializer_list<std::pair<const std::string, Node>> init_list) noexcept
                :data(Map{init_list}) {}


            /**
             * @brief Получение дочернего узла по ключу.
             * @param key Ключ узла.
             * @return Ссылка на дочерний узел.
             */
            Node &operator[](const std::string &key);

            /**
             * @brief Получение дочернего узла по индексу.
             * @param index Индекс узла.
             * @return Ссылка на дочерний узел.
             */
            auto operator[](const size_t index);

            /**
             * @brief Получение значения узла указанного типа.
             * @tparam T Тип значения.
             * @return Значение узла.
             */
            template <typename T>
            T get() const
            {
                if (isValue())
                {
                    return std::get<T>(std::get<Value>(data));
                }
                else
                {
                    throw std::runtime_error("This node isnt value");
                }
            }

            /**
             * @brief Получение значения узла типа Array.
             * @return Узел типа Array.
             */
            template <>
            Array get<Array>() const
            {
                if (isArray())
                {
                    return std::get<Array>(data);
                }
                else
                {
                    throw std::runtime_error("This node isnt array");
                }
            }

            /**
             * @brief Получение значения узла типа Map.
             * @return Узел типа Map.
             */
            template <>
            Map get<Map>() const
            {
                if (isObject())
                {
                    return std::get<Map>(data);
                }
                else
                {
                    throw std::runtime_error("This node isnt map");
                }
            }

            template<typename T>
            std::vector<T> getArray() const
            {
                std::vector<T> result;

                if(isArray())
                {
                    for(const auto& node : get<Array>())
                    {
                        if(node.isValue())
                        {
                            result.push_back(node.get<T>());
                        }
                    }
                }
                else
                {
                    throw std::runtime_error("This node isnt array");
                }
                return result;
            }

            template<>
            std::vector<std::filesystem::path> getArray() const
            {
                std::vector<std::filesystem::path> result;

                if(isArray())
                {
                    for(const auto& node : get<Array>())
                    {
                        if(node.isValue())
                        {
                            result.push_back(std::filesystem::path(node.get<std::string>()));
                        }
                    }
                }
                else
                {
                    throw std::runtime_error("This node isnt array");
                }
                return result;

            }

            /**
             * @brief Проверка, является ли узел объектом.
             * @return true, если узел является объектом, иначе false.
             */
            inline constexpr bool isObject() const noexcept
            {
                return std::holds_alternative<Map>(data);
            }

            /**
             * @brief Проверка, является ли узел значением.
             * @return true, если узел является значением, иначе false.
             */
            inline constexpr bool isValue() const noexcept
            {
                return std::holds_alternative<Value>(data);
            }

            /**
             * @brief Проверка, является ли узел массивом.
             * @return true, если узел является массивом, иначе false.
             */
            inline constexpr bool isArray() const noexcept
            {
                return std::holds_alternative<Array>(data);
            }
            
            /**
             * @brief Проверка, является ли узел строкой.
             * @return true, если узел является строкой, иначе false.
             */
            inline constexpr bool isString() const
            {
                return isValue() && std::holds_alternative<std::string>(std::get<Value>(data));
            }

            /**
             * @brief Преобразование узла в строку .
             * @return Строковое представление узла.
             */
            inline std::string stringify() const
            {
                return std::visit([this](const auto &arg)
                                  { return stringifyData(arg); }, data);
            }

        private:

            inline std::string stringifyData(const Value &val) const
            {
                return std::visit([this](const auto &arg)
                                  { return toString(arg); }, val);
            }

            inline std::string stringifyData(const Map &map) const
            {
                return toString(map);
            }
            inline std::string stringifyData(const Array &arr) const
            {
                return toString(arr);
            }

            template <typename T>
            std::string toString(const T &value) const
            {
                std::ostringstream oss;
                oss << value;
                return oss.str();
            }

            // Специализация для std::string (чтобы не добавлялись кавычки при записи)
            inline std::string toString(const std::string &value) const
            {
                return value;
            }

            /**
            * @todo Класс должен выкинуть исключение
            */
            inline std::string toString(const Map &map) const
            {
                return {};
            }

            inline std::string toString(const Array &arr) const
            {
                std::ostringstream oss;

                for (const auto &i : arr)
                {
                    oss << i.stringify();
                }
                return oss.str();
            }

            inline std::string toString(std::nullptr_t) const
            {
                return "null";
            }
            inline std::string toString(const bool val) const
            {
                return val ? "true" : "false";
            }

            std::variant<Map, Value, Array> data;
        };
    };
};

#endif