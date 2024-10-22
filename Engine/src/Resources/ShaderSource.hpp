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

        private:
            std::string source;
        };
    };
};

#endif