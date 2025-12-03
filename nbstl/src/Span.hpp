#ifndef NBSTL_SRC_SPAN_HPP
#define NBSTL_SRC_SPAN_HPP

#include "TypeTraits.hpp"
#include "Types.hpp"
#include "Array.hpp"

namespace nbstl
{
    template <class T>
    class Span
    {
    public:
        using element_type = T;
        using value_type = nbstl::removeConstVolatile<T>;
        using pointer = T*;
        using reference = T&;
        using iterator = T*;
        using const_iterator = const T*;
        using size_type = Size;

    private:
        pointer ptr = nullptr;
        size_type count = 0;

    public:

        constexpr Span() noexcept = default;

        constexpr Span(pointer p, size_type n) noexcept
            : ptr(p), count(n) {
        }

        template <size_type N>
        constexpr Span(T(&arr)[N]) noexcept
            : ptr(arr), count(N) {
        }

        // For mutable Array
        template <size_type N>
        constexpr Span(nbstl::Array<T, N>& arr) noexcept
            : ptr(arr.data()), count(static_cast<size_type>(N)) {
        }

        // For const Array
        template <size_type N>
        constexpr Span(const nbstl::Array<T, N>& arr) noexcept
            : ptr(arr.data()), count(static_cast<size_type>(N)) {
        }

        // For Array with convertible types
        template <typename U, size_type N>
        constexpr Span(nbstl::Array<U, N>& arr) noexcept
            : ptr(arr.data()), count(static_cast<size_type>(N)) {
        }

        template <typename U, size_type N>
        constexpr Span(const nbstl::Array<U, N>& arr) noexcept
            : ptr(arr.data()), count(static_cast<size_type>(N)) {
        }

        constexpr operator Span<const T>() const noexcept
        {
            return Span<const T>(ptr, count);
        }


        constexpr iterator begin() noexcept { return ptr; }
        constexpr const_iterator begin() const noexcept { return ptr; }
        constexpr const_iterator cbegin() const noexcept { return ptr; }

        constexpr iterator end() noexcept { return ptr + count; }
        constexpr const_iterator end() const noexcept { return ptr + count; }
        constexpr const_iterator cend() const noexcept { return ptr + count; }

        constexpr reference operator[](size_type i) noexcept { return ptr[i]; }
        constexpr const T& operator[](size_type i) const noexcept { return ptr[i]; }

        constexpr pointer data() noexcept { return ptr; }
        constexpr const pointer data() const noexcept { return ptr; }

        constexpr size_type size() const noexcept { return count; }
        constexpr bool empty() const noexcept { return count == 0; }

        constexpr Span subspan(size_type offset, size_type n = static_cast<size_type>(-1)) const noexcept
        {
            if (offset > count) return Span();
            if (n == static_cast<size_type>(-1) || offset + n > count)
                n = count - offset;
            return Span(ptr + offset, n);
        }
    };

};

#endif