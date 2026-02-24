#ifndef NBSTL_SRC_REFLECTION_REFLECTION_HPP
#define NBSTL_SRC_REFLECTION_REFLECTION_HPP

#include "Types.hpp"
#include "Vector.hpp"

#include <unordered_map>

#define REFLECT_TYPE(TYPE) TypeInfo TYPE##_Type = {#TYPE, sizeof(TYPE), {}}

#define REFLECT_FIELD(TYPE, FIELD, FIELD_TYPE)                                                     \
    TYPE##_Type.fields.push_back({#FIELD, offsetof(TYPE, FIELD), &FIELD_TYPE##_Type});

namespace nbui
{
    struct TypeInfo
    {
        const char* name;
        Size size;

        Vector<FieldInfo> fields;
    };

    struct FieldInfo
    {
        const char* name;
        Size offset;
        const TypeInfo* type;
    };

    void* getFieldPtr(
        void* object,
        const FieldInfo& field
    )
    {
        return static_cast<char*>(object) + field.offset;
    }

    class TypeRegistry
    {
    public:
        static TypeRegistry& instance()
        {
            static TypeRegistry registry;
            return registry;
        }

        void registerType(TypeInfo* type)
        {
            types[type->name] = type;
        }

        TypeInfo* get(const std::string& name)
        {
            auto it = types.find(name);
            if (it != types.end())
            {
                return it->second;
            }

            return nullptr;
        }

    private:
        std::unordered_map<std::string, TypeInfo*> types;
    };
}

#endif
