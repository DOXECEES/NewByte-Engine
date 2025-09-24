#ifndef SRC_RESOURCES_IRESOURCE_HPP
#define SRC_RESOURCES_IRESOURCE_HPP

#include "Core.hpp"
#include "Error/ErrorManager.hpp"

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


		template<typename T>
        T* resourceTo(IResource* resource)
        {
            T* castedResource = dynamic_cast<T*>(resource);
            if (!castedResource) NB_UNLIKELY
            {
                Error::ErrorManager::instance().report(Error::Type::FATAL, "Failed to cast resource to type")
                    .with("type_name", typeid(T).name());
                
                return nullptr;
            }

            return castedResource;
        }
    };
};

#endif