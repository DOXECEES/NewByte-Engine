#ifndef NBSTL_SRC_ARRAY_HPP
#define NBSTL_SRC_ARRAY_HPP

#include "Types.hpp"

namespace nbstl
{
    template <class T, Size N>
    class Array
    {
    public:
        using value_type = T;
        using size_type = Size;
        using reference = T&;
        using const_reference = const T&;
        using pointer = T*;
        using const_pointer = const T*;
        using iterator = T*;
        using const_iterator = const T*;

    private:
        T data_[N == 0 ? 1 : N]; 

    public:
        constexpr reference operator[](size_type i) noexcept {
            return data_[i];
        }

        constexpr const_reference operator[](size_type i) const noexcept {
            return data_[i];
        }

        constexpr reference front() noexcept { return data_[0]; }
        constexpr const_reference front() const noexcept { return data_[0]; }

        constexpr reference back() noexcept { return data_[N == 0 ? 0 : N - 1]; }
        constexpr const_reference back() const noexcept { return data_[N == 0 ? 0 : N - 1]; }

        constexpr pointer data() noexcept { return data_; }
        constexpr const_pointer data() const noexcept { return data_; }

        constexpr iterator begin() noexcept { return data_; }
        constexpr const_iterator begin() const noexcept { return data_; }
        constexpr const_iterator cbegin() const noexcept { return data_; }

        constexpr iterator end() noexcept { return data_ + N; }
        constexpr const_iterator end() const noexcept { return data_ + N; }
        constexpr const_iterator cend() const noexcept { return data_ + N; }

        constexpr size_type size() const noexcept { return N; }
        constexpr bool empty() const noexcept { return N == 0; }

        constexpr void fill(const T& value) noexcept
        {
            for (size_type i = 0; i < N; i++)
            {
                data_[i] = value;
            }
        }
    };

};

#endif