#ifndef SRC_RESOURCES_IRESOURCE_HPP
#define SRC_RESOURCES_IRESOURCE_HPP

#include "../Loaders/ILoader.hpp"

namespace nb
{
    namespace Resource
    {
        class IResource
        {
        public:
            virtual ~IResource() = default;
        };
    };
};

#endif