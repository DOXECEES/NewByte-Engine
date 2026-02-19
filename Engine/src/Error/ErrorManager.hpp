#ifndef SRC_ERROR_ERRORMANAGER_HPP
#define SRC_ERROR_ERRORMANAGER_HPP

#include <string>
#include <unordered_map>
#include <chrono>
#include <queue>





template<typename T>
inline std::string toString(const T& val) noexcept
{
    std::ostringstream oss;
    oss << val;
    return oss.str();
}


namespace nb
{
    namespace Error
    {
        class IErrorPrinter;
        // TODO: stringify macro 
        enum class Type : uint8_t
        {
            NONE    = 0,
            INFO    = 1 << 0,
            WARNING = 1 << 1,
            FATAL   = 1 << 2,

            ALL     = static_cast<uint8_t>(0xFF)
        };


        inline bool operator&(Type lhs, Type rhs) noexcept
        {
            return static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs);
        }

        inline Type operator|(Type lhs, Type rhs) noexcept
        {
            return static_cast<Type>(
                static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs)
            );
        }

        struct ErrorMessage
        {
            Type                                                    type;
            std::chrono::time_point<std::chrono::system_clock>      time;
            std::string_view                                        message;
            std::unordered_map<std::string, std::string>            data;
        };


        class ErrorManager
        {
        public:
            class ProxyMessage
            {
            public:
                ProxyMessage(Type errorType, std::string_view message) noexcept;

                ~ProxyMessage() noexcept;

                template<typename T>
                ProxyMessage& with(std::string_view key, const T& value) noexcept
                {
                    message.data[key.data()] = toString(value);
                    return *this;
                }

            private:
                ErrorMessage message;
            };


            static ErrorManager& instance() noexcept;

            ProxyMessage report(Type errorType, std::string_view message) noexcept;

            std::vector<ErrorMessage> getAllMessages() noexcept;
            std::vector<ErrorMessage> getOnly(Type type) noexcept;
            ErrorMessage peek() const noexcept;
            ErrorMessage get() noexcept;
            
            inline bool hasMessages() const noexcept { return isMessageReceived; };
            void setPrinter(IErrorPrinter* printerPtr) { this->printer = printerPtr; }

        private:
            ErrorManager() noexcept = default;
            ~ErrorManager() = default;
            ErrorManager(const ErrorManager&) = delete;
            ErrorManager& operator=(const ErrorManager&) = delete;
            ErrorManager(ErrorManager&&) = delete;
            ErrorManager& operator=(ErrorManager&&) = delete;

            void insert(const ErrorMessage& message) noexcept;

           

        private:
            bool                        isMessageReceived = false;
            std::queue<ErrorMessage>    messages;
            
            IErrorPrinter* printer = nullptr;
        
        };
    };
};


#endif
