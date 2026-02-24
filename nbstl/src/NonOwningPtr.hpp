#ifndef NBSTL_SRC_NONOWNINGPTR_HPP
#define NBSTL_SRC_NONOWNINGPTR_HPP

#include <NbCore.hpp>

namespace nbstl 
{
    template<typename T>
    class NonOwningPtr
    {
    public:
        constexpr NonOwningPtr() noexcept = default;
        constexpr NonOwningPtr(T* rawPtr) noexcept : ptr(rawPtr) {}

        constexpr NonOwningPtr(const NonOwningPtr&) noexcept = default;
        constexpr NonOwningPtr& operator=(const NonOwningPtr&) noexcept = default;

        constexpr NonOwningPtr(NonOwningPtr&& other) noexcept : ptr(other.ptr)
        {
            other.ptr = nullptr;
        }

        template<typename U>
        constexpr NonOwningPtr& operator=(U* otherPtr) noexcept
        {
            ptr = static_cast<T*>(otherPtr); 
            return *this;
        }

        constexpr NonOwningPtr& operator=(NonOwningPtr&& other) noexcept
        {
            if (this != &other)
            {
                ptr = other.ptr;
                other.ptr = nullptr;
            }
            return *this;
        }

        constexpr T* get() const noexcept { return ptr; }

        constexpr T& operator*() const noexcept
        {
            NB_ASSERT(ptr, "Dereferencing null NonOwningPtr");
            return *ptr;
        }

        constexpr T* operator->() const noexcept
        {
            NB_ASSERT(ptr, "Dereferencing null NonOwningPtr");
            return ptr;
        }

        constexpr explicit operator bool() const noexcept { return ptr != nullptr; }

        constexpr void reset(T* newPtr = nullptr) noexcept { ptr = newPtr; }

        constexpr bool operator==(const NonOwningPtr& other) const noexcept { return ptr == other.ptr; }
        constexpr bool operator!=(const NonOwningPtr& other) const noexcept { return ptr != other.ptr; }

        constexpr bool operator==(std::nullptr_t) const noexcept { return ptr == nullptr; }
        constexpr bool operator!=(std::nullptr_t) const noexcept { return ptr != nullptr; }

    private:
        T* ptr = nullptr;

    };
}; 

#endif 