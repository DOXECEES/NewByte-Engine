#ifndef SRC_SERIALIZE_ISERIALIZABLE_HPP
#define SRC_SERIALIZE_ISERIALIZABLE_HPP

#include "IArchive.hpp"

namespace nb::Serialize
{
    class ISerializable
    {
    public:
        virtual void serialize(IArchive* archive) noexcept = 0;
        virtual void deserialize(IArchive* archive) noexcept = 0;
    };

};

#endif
