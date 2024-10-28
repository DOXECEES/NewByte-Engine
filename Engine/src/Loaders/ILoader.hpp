#ifndef SRC_LOADERS_ILOADER_HPP
#define SRC_LOADERS_ILOADER_HPP

#include <filesystem>

namespace nb
{
    /// @todo Все лоадеры должны наследовать ILoader  

    namespace Loaders
    {

        /// @brief Представляет собой интерфейс для чтения файлов
        class IReadable 
        {
        public:
            virtual void readFromFile(const std::filesystem::path& path) = 0;
                //readFromMemory(char* , size)-> cast = 0 


        private:
        };

        /// @brief Представляет собой интерфейс для записи файлов
        class IWriteable 
        {
        public:

            virtual void writeToFile(const std::filesystem::path& path) = 0;
        
        private:

        };

    };
};

#endif