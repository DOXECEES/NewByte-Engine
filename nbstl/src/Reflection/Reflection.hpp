#ifndef NBSTL_SRC_REFLECTION_REFLECTION_HPP
#define NBSTL_SRC_REFLECTION_REFLECTION_HPP

#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <vector>
#include <functional>

namespace nb::Reflect
{

    struct TypeInfo;

    template <typename T> struct Reflect
    {
        static inline bool isInternal = false;
    };


    template <typename T, typename = void> struct hasReflection : std::false_type
    {
    };

    template <typename T>
    struct hasReflection<T, std::void_t<decltype(Reflect<T>::fields)>> : std::true_type
    {
    };

    template <typename T, typename = void>
    struct hasEnumReflection : std::false_type
    {
    };

    template <typename T>
    struct hasEnumReflection<T, std::void_t<decltype(Reflect<T>::values)>> : std::true_type
    {
    };

    template <typename T, typename Enable = void>
    struct ResourceLoader
    {
        // пусто по умолчанию
    };

    // Проверка наличия ResourceLoader
    template <typename T, typename = void>
    struct hasResourceLoader : std::false_type
    {
    };

    template <typename T>
    struct hasResourceLoader<T, std::void_t<decltype(ResourceLoader<T>::load)>> : std::true_type
    {
    };

    // Проверка наличия getPath
    template <typename T, typename = void>
    struct hasResourceGetPath : std::false_type
    {
    };

    template <typename T>
    struct hasResourceGetPath<T, std::void_t<decltype(ResourceLoader<T>::getPath)>> : std::true_type
    {
    };


    struct EnumValueInfo
    {
        const char* name;
        int value;
    };

    struct FieldInfo
    {
        const char* name;
        size_t offset;
        TypeInfo* type;
        bool (*visibleIf)(void* object) = nullptr;
        float step = 0.1f;

        std::function<std::string(void*)> getResourcePath = nullptr;
    };

    struct TypeInfo
    {
        const char* name;
        size_t size;
        std::vector<FieldInfo> fields;
        bool isInternal = false;

        bool isEnum = false;
        std::vector<EnumValueInfo> enumValues;

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

            if constexpr (hasEnumReflection<T>::value)
            {
                type.name = Reflect<T>::name;
                type.size = sizeof(T);
                type.isEnum = true;
                type.enumValues = Reflect<T>::values;
            }
            else if constexpr (hasReflection<T>::value)
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
                                info.step = fieldDesc.step;

                                if constexpr (hasResourceGetPath<MemberType>::value)
                                {
                                    info.getResourcePath = [](void* ptr) -> std::string
                                    {
                                        return ResourceLoader<MemberType>::getPath(ptr);
                                    };
                                }

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

    template <>
    inline TypeInfo* getType<uint8_t>()
    {
        static TypeInfo type = {"uint8_t", sizeof(uint8_t), {}, false};
        return &type;
    }

    template <>
    inline TypeInfo* getType<std::string>()
    {
        static TypeInfo type = {"std::string", sizeof(std::string), {}, false};
        return &type;
    }

  template <typename MemberType>
    std::string getResourcePath(void* fieldPtr)
    {
        if constexpr (nb::Reflect::hasResourceGetPath<MemberType>::value)
        {
            return nb::Reflect::ResourceLoader<MemberType>::getPath(fieldPtr);
        }
        else
        {
            return "";
        }
    }

    // Перегрузка для FieldInfo
    template <typename Class>
    std::string getResourcePathFromField(
        void* object,
        FieldInfo* field
    )
    {
        // смещение уже хранится в field->offset
        void* fieldPtr = reinterpret_cast<uint8_t*>(object) + field->offset;

        // MemberType можно получить через рефлексию:
        for (auto& f : nb::Reflect::getType<Class>()->fields)
        {
            if (&f == field)
            {
                using MemberType = std::remove_pointer_t<decltype(((Class*)0)->*reinterpret_cast<nb::Reflect::FieldDesc<Class, decltype(*field) >*>(&f)->member)>;
                return getResourcePath<MemberType>(fieldPtr);
            }
        }

        return "";
    }

    





#define NB_REFLECT_STRUCT(TYPE, ...)                                                               \
    template <> struct nb::Reflect::Reflect<TYPE>                                                  \
    {                                                                                              \
        static inline const char* name = #TYPE;                                                 \
        static inline auto fields = std::make_tuple(__VA_ARGS__);   \
        static inline bool isInternal = false;   \
    };

#define NB_REFLECT_STRUCT_CUSTOM_NAME(TYPE, NAME, ...)                                                               \
    template <> struct nb::Reflect::Reflect<TYPE>                                                  \
    {                                                                                              \
        static inline const char* name = NAME;                                                 \
        static inline auto fields = std::make_tuple(__VA_ARGS__);                               \
        static inline bool isInternal = false;                                                  \
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


#define NB_ENUM_VALUE(VALUE)                                                                       \
    nb::Reflect::EnumValueInfo                                                                     \
    {                                                                                              \
        #VALUE, (int)VALUE                                                                         \
    }

#define NB_REFLECT_ENUM(TYPE, ...)                                                                 \
    template <>                                                                                    \
    struct nb::Reflect::Reflect<TYPE>                                                              \
    {                                                                                              \
        static inline const char* name = #TYPE;                                                    \
        static inline auto values = std::vector<nb::Reflect::EnumValueInfo>{__VA_ARGS__};          \
        static inline bool isInternal = false;                                                     \
    };

#define NB_REFLECT_PTR(TYPE, NAME)                                                        \
    template <>                                                                                    \
    struct nb::Reflect::Reflect<TYPE>                                                              \
    {                                                                                              \
        static inline const char* name = NAME;                                                     \
        static inline auto fields = std::make_tuple();                                             \
        static inline bool isInternal = true;                                                      \
    };

#define NB_REFLECT_RESOURCE_PTR(TYPE, NAME, LOADER)                                                \
    template <>                                                                                    \
    struct nb::Reflect::ResourceLoader<TYPE>                                                       \
    {                                                                                              \
        static void load(                                                                          \
            void* fieldPtr,                                                                        \
            const std::string& path                                                                \
        )                                                                                          \
        {                                                                                          \
            LOADER(reinterpret_cast<TYPE*>(fieldPtr), path);                                       \
        }                                                                                          \
                                                                                                   \
        static std::string getPath(void* fieldPtr)                                                 \
        {                                                                                          \
            TYPE* ptr = reinterpret_cast<TYPE*>(fieldPtr);                                         \
            if (!ptr)                                                                              \
            {                                                                                      \
                return "";                                                                         \
            }                                                                                      \
                                                                                                   \
            if (*ptr)                                                                              \
            {                                                                                      \
                return (*ptr)->getPath();                                                          \
            }                                                                                      \
                                                                                                   \
            return "";                                                                             \
        }                                                                                          \
    };                                                                                             \
                                                                                                   \
  
};

#endif