// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "ErrorManager.hpp"

namespace nb
{
    namespace Error
    {
        ErrorManager& ErrorManager::instance() noexcept
        {
            static ErrorManager instance;
            return instance;
        }
        ErrorManager::ProxyMessage ErrorManager::report(Type errorType, std::string_view message) noexcept
        {
            isMessageReceived = true;
            return ProxyMessage(errorType, message);
        }

        std::vector<ErrorMessage> ErrorManager::getAllMessages() noexcept
        {
            std::vector<ErrorMessage> result;
            size_t size = messages.size();
            result.reserve(size);

            for (size_t i = 0; i < size; i++)
            {
                result.push_back(messages.front());
                messages.pop();
            }

            isMessageReceived = false;

            return result;
        }

        std::vector<ErrorMessage> ErrorManager::getOnly(Type type) noexcept
        {
            std::vector<ErrorMessage> result;
            size_t size = messages.size();
            //result.reserve(size);

            for (size_t i = 0; i < size; i++)
            {
                if (messages.front().type & type)
                {
                    result.push_back(messages.front());
                }
                messages.pop();
            }

            isMessageReceived = false;
            
            return result;
        }

        ErrorMessage ErrorManager::peek() const noexcept
        {
            return messages.front();
        }

        ErrorMessage ErrorManager::get() noexcept
        {
            ErrorMessage msg = messages.front();
            messages.pop();
            return msg;
        }

        void ErrorManager::insert(const ErrorMessage& message) noexcept
        {
            messages.push(message);
        }

        ErrorManager::ProxyMessage::ProxyMessage(Type errorType, std::string_view message) noexcept
        {
            this->message.type = errorType;
            this->message.time = std::chrono::system_clock::now();
            this->message.message = message;
        }

        ErrorManager::ProxyMessage::~ProxyMessage() noexcept
        {
            ErrorManager::instance().insert(message);
        }
    };
};