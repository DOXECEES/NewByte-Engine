#ifndef SRC_RESOURCES_SHADERSOURCE_HPP
#define SRC_RESOURCES_SHADERSOURCE_HPP

#include <string>

#include "IResource.hpp"

namespace nb
{
    namespace Resource
    {
        class ShaderSource : public IResource
        {
        public:
            ShaderSource() noexcept = default;
            ShaderSource(const std::string &str) noexcept;
            virtual ~ShaderSource() = default;
            
            //void readFromFile(const std::filesystem::path& path) override{};
            //void writeToFile(const std::filesystem::path &path) override {};

        private:
            std::string source;
        };
    };
};

#endif