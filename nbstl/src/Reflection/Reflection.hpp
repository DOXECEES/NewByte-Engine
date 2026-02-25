#ifndef NBSTL_SRC_REFLECTION_REFLECTION_HPP
#define NBSTL_SRC_REFLECTION_REFLECTION_HPP

#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace nb::Reflect
{

    struct TypeInfo;

    template <typename T> struct Reflect
    {
        static constexpr bool isInternal = false;
    };

    template <typename T, typename = void> struct hasReflection : std::false_type
    {
    };

    template <typename T>
    struct hasReflection<T, std::void_t<decltype(Reflect<T>::fields)>> : std::true_type
    {
    };



    struct FieldInfo
    {
        const char* name;
        size_t offset;
        TypeInfo* type;
        bool (*visibleIf)(void* object) = nullptr;
        float step = 0.1f;
    };

    struct TypeInfo
    {
        const char* name;
        size_t size;
        std::vector<FieldInfo> fields;
        bool isInternal = false;
    };


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
            return it != types.end() ? it->second : nullptr;
        }

    private:
        std::unordered_map<std::string, TypeInfo*> types;
    };


    template <typename Class, typename Member> struct FieldDesc
    {
        const char* name;
        Member Class::* member;
        bool (*visibleIfRaw)(void*) = nullptr;
        float step = 0.1f;
    };


    template <typename T> TypeInfo* getType();

    template <typename T> TypeInfo* buildType()
    {
        static TypeInfo type;
        static bool initialized = false;

        if (!initialized)
        {
            type.isInternal = Reflect<T>::isInternal;

            if constexpr (hasReflection<T>::value)
            {
                type.name = Reflect<T>::name;
                type.size = sizeof(T);

                std::apply(
                    [&](auto&&... fieldDesc)
                    {
                        (
                            [&]()
                            {
                                using MemberType = std::remove_reference_t<
                                    decltype(((T*)0)->*(fieldDesc.member))>;

                                size_t offset = reinterpret_cast<size_t>(
                                    &(reinterpret_cast<T const volatile*>(0)->*(fieldDesc.member))
                                );

                                FieldInfo info;
                                info.name = fieldDesc.name;
                                info.offset = offset;
                                info.type = getType<MemberType>();
                                info.visibleIf = fieldDesc.visibleIfRaw;

                                type.fields.push_back(info);
                            }(),
                            ...);
                    },
                    Reflect<T>::fields
                );
            }
            else
            {
                type.name = "Primitive";
                type.size = sizeof(T);
            }

            

            initialized = true;
            TypeRegistry::instance().registerType(&type);
        }

        return &type;
    }

    template <typename T> TypeInfo* getType()
    {
        return buildType<T>();
    }


    template <> inline TypeInfo* getType<float>()
    {
        static TypeInfo type = {"float", sizeof(float), {}, false};
        return &type;
    }

    template <> inline TypeInfo* getType<int>()
    {
        static TypeInfo type = {"int", sizeof(int), {}, false};
        return &type;
    }

    template <> inline TypeInfo* getType<bool>()
    {
        static TypeInfo type = {"bool", sizeof(bool), {}, false};
        return &type;
    }


#define NB_REFLECT_STRUCT(TYPE, ...)                                                               \
    template <> struct nb::Reflect::Reflect<TYPE>                                                  \
    {                                                                                              \
        static constexpr const char* name = #TYPE;                                                 \
        static constexpr auto fields = std::make_tuple(__VA_ARGS__);   \
        static constexpr bool isInternal = false;   \
    };

#define NB_REFLECT_STRUCT_CUSTOM_NAME(TYPE, NAME, ...)                                                               \
    template <> struct nb::Reflect::Reflect<TYPE>                                                  \
    {                                                                                              \
        static constexpr const char* name = NAME;                                                 \
        static constexpr auto fields = std::make_tuple(__VA_ARGS__);                               \
        static constexpr bool isInternal = false;                                                  \
    };

#define NB_FIELD(CLASS, FIELD_NAME)                                                          \
    nb::Reflect::FieldDesc<CLASS, std::remove_reference_t<decltype(CLASS::FIELD_NAME)>>            \
    {                                                                                              \
        #FIELD_NAME, &CLASS::FIELD_NAME, nullptr, 0.1f                                                   \
    }

#define NB_FIELD_STEP(CLASS, FIELD_NAME, STEP)                                                                \
    nb::Reflect::FieldDesc<CLASS, std::remove_reference_t<decltype(CLASS::FIELD_NAME)>>{           \
        #FIELD_NAME, &CLASS::FIELD_NAME, nullptr, (float)STEP                                             \
    }

#define NB_FIELD_VISIBLE_IF(CLASS, FIELD_NAME, METHOD, STEP)                                             \
    nb::Reflect::FieldDesc<CLASS, std::remove_reference_t<decltype(CLASS::FIELD_NAME)>>            \
    {                                                                                              \
        #FIELD_NAME, &CLASS::FIELD_NAME, [](void* obj) -> bool                                     \
        {                                                                                          \
            return static_cast<CLASS*>(obj)->METHOD();                                             \
        },                                                                                          \
        (float)STEP                                                                                \
    }

#define NB_REFLECT_INTERNAL_STRUCT(TYPE, ...)                                                               \
    template <> struct nb::Reflect::Reflect<TYPE>                                                  \
    {                                                                                              \
        static constexpr const char* name = #TYPE;                                                 \
        static constexpr auto fields = std::make_tuple(__VA_ARGS__);                               \
        static constexpr bool isInternal = true;                                                  \
    };


};

#endif