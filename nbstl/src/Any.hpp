//#ifndef NBSTL_SRC_ANY_HPP
//#define NBSTL_SRC_ANY_HPP
//
//
//#include <utility>
//#include <new>
//#include <cstddef>
//#include <type_traits>
//
//namespace nbstl
//{
//    template<std::size_t BufferSize = 32>
//    class Any
//    {
//    private:
//
//        static constexpr std::size_t BufferAlign =
//            alignof(std::max_align_t);
//
//        using Storage = alignas(BufferAlign) unsigned char[BufferSize];
//
//        struct VTable
//        {
//            void (*destroy)(void*) noexcept;
//            void (*copy)(void*, const void*);
//            void (*move)(void*, void*) noexcept;
//            const void* (*type)() noexcept;
//        };
//
//        template<typename T>
//        struct TypeTag
//        {
//            static const void* id() noexcept
//            {
//                static int value;
//                return &value;
//            }
//        };
//
//        template<typename T>
//        static void destroy_inline(void* ptr) noexcept
//        {
//            reinterpret_cast<T*>(ptr)->~T();
//        }
//
//        template<typename T>
//        static void destroy_heap(void* ptr) noexcept
//        {
//            delete reinterpret_cast<T*>(ptr);
//        }
//
//        template<typename T>
//        static void copy_inline(void* dst, const void* src)
//        {
//            new (dst) T(*reinterpret_cast<const T*>(src));
//        }
//
//        template<typename T>
//        static void copy_heap(void* dst, const void* src)
//        {
//            *reinterpret_cast<void**>(dst) =
//                new T(*reinterpret_cast<const T*>(
//                    *reinterpret_cast<void* const*>(src)));
//        }
//
//        template<typename T>
//        static void move_inline(void* dst, void* src) noexcept
//        {
//            new (dst) T(std::move(*reinterpret_cast<T*>(src)));
//            destroy_inline<T>(src);
//        }
//
//        template<typename T>
//        static void move_heap(void* dst, void* src) noexcept
//        {
//            *reinterpret_cast<void**>(dst) =
//                *reinterpret_cast<void**>(src);
//            *reinterpret_cast<void**>(src) = nullptr;
//        }
//
//        template<typename T>
//        static const VTable* get_inline_vtable() noexcept
//        {
//            static const VTable table =
//            {
//                &destroy_inline<T>,
//                &copy_inline<T>,
//                &move_inline<T>,
//                &TypeTag<T>::id
//            };
//            return &table;
//        }
//
//        template<typename T>
//        static const VTable* get_heap_vtable() noexcept
//        {
//            static const VTable table =
//            {
//                &destroy_heap<T>,
//                &copy_heap<T>,
//                &move_heap<T>,
//                &TypeTag<T>::id
//            };
//            return &table;
//        }
//
//    private:
//
//        const VTable* vtable = nullptr;
//        bool is_heap = false;
//
//        union
//        {
//            Storage buffer;
//            void* heap_ptr;
//        };
//
//    public:
//
//        Any() noexcept = default;
//
//        ~Any()
//        {
//            reset();
//        }
//
//        Any(const Any& other)
//        {
//            if (other.vtable)
//            {
//                other.vtable->copy(
//                    data_ptr(),
//                    other.data_ptr()
//                );
//
//                vtable = other.vtable;
//                is_heap = other.is_heap;
//            }
//        }
//
//        Any(Any&& other) noexcept
//        {
//            if (other.vtable)
//            {
//                other.vtable->move(
//                    data_ptr(),
//                    other.data_ptr()
//                );
//
//                vtable = other.vtable;
//                is_heap = other.is_heap;
//
//                other.vtable = nullptr;
//                other.is_heap = false;
//            }
//        }
//
//        template<
//            typename T,
//            typename Decayed = std::decay_t<T>,
//            typename = std::enable_if_t<
//            !std::is_same_v<Decayed, Any>>
//            >
//            Any(T&& value)
//        {
//            emplace<Decayed>(std::forward<T>(value));
//        }
//
//        template<typename T, typename... Args>
//        void emplace(Args&&... args)
//        {
//            reset();
//
//            using U = std::decay_t<T>;
//
//            if constexpr (
//                sizeof(U) <= BufferSize &&
//                alignof(U) <= BufferAlign
//                )
//            {
//                new (buffer) U(std::forward<Args>(args)...);
//                vtable = get_inline_vtable<U>();
//                is_heap = false;
//            }
//            else
//            {
//                heap_ptr =
//                    new U(std::forward<Args>(args)...);
//                vtable = get_heap_vtable<U>();
//                is_heap = true;
//            }
//        }
//
//        void reset() noexcept
//        {
//            if (!vtable)
//            {
//                return;
//            }
//
//            vtable->destroy(data_ptr());
//            vtable = nullptr;
//            is_heap = false;
//        }
//
//        bool has_value() const noexcept
//        {
//            return vtable != nullptr;
//        }
//
//        template<typename T>
//        T* cast() noexcept
//        {
//            if (!vtable ||
//                vtable->type() != TypeTag<T>::id())
//            {
//                return nullptr;
//            }
//
//            if (is_heap)
//            {
//                return reinterpret_cast<T*>(heap_ptr);
//            }
//
//            return reinterpret_cast<T*>(buffer);
//        }
//
//        template<typename T>
//        const T* cast() const noexcept
//        {
//            return const_cast<Any*>(this)->cast<T>();
//        }
//
//    private:
//
//        void* data_ptr() noexcept
//        {
//            return is_heap
//                ? static_cast<void*>(&heap_ptr)
//                : static_cast<void*>(buffer);
//        }
//
//        const void* data_ptr() const noexcept
//        {
//            return is_heap
//                ? static_cast<const void*>(&heap_ptr)
//                : static_cast<const void*>(buffer);
//        }
//    };
//
//};
//
//#endif