#ifndef NBSTL_SRC_UTILS_HPP
#define NBSTL_SRC_UTILS_HPP

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <immintrin.h>

struct StringPosition
{
    constexpr bool isValid() const noexcept { return found; }
    constexpr operator bool() const noexcept { return isValid(); }
    constexpr size_t getPosition() const noexcept { return position; }

    constexpr StringPosition() noexcept : found(false), position(0) {}
    constexpr explicit StringPosition(size_t pos) noexcept : found(true), position(pos) {}

private:
    bool found;
    size_t position;
};

inline StringPosition bmhFindFast(const char* __restrict text, size_t n,
    const char* __restrict pattern, size_t m) noexcept
{
    if (m == 0 || n < m)
        return {};

    // Быстрый путь для коротких паттернов (<=8)
    if (m <= 8)
    {
        uint64_t mask = 0;
        std::memcpy(&mask, pattern, m);
        for (size_t i = 0; i + m <= n; ++i)
        {
            uint64_t chunk = 0;
            std::memcpy(&chunk, text + i, m);
            if (chunk == mask)
                return StringPosition(i);
        }
        return {};
    }

    alignas(64) uint8_t shift[256];
    const uint8_t* pat = reinterpret_cast<const uint8_t*>(pattern);
    const uint8_t last = pat[m - 1];

    for (size_t i = 0; i < 256; ++i)
        shift[i] = static_cast<uint8_t>(m);

    for (size_t i = 0; i < m - 1; ++i)
        shift[pat[i]] = static_cast<uint8_t>(m - i - 1);

    const char* end = text + n - m;
    const char* ptr = text;

    __m128i lastv = _mm_set1_epi8(static_cast<char>(last));

    while (ptr <= end)
    {
        // Сравниваем по 16 байт сразу
        __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr + m - 16));
        __m128i eq = _mm_cmpeq_epi8(chunk, lastv);
        unsigned mask = static_cast<unsigned>(_mm_movemask_epi8(eq));

        while (mask)
        {
            unsigned long offset;
            _BitScanForward(&offset, mask);  // ← исправленный вызов
            size_t pos = (m - 16) + offset;

            if (pos < m && std::memcmp(ptr + pos - (m - 1), pattern, m) == 0)
                return StringPosition(ptr + pos - (m - 1) - text);

            mask &= mask - 1; // сбрасываем младший найденный бит
        }

        ptr += shift[static_cast<uint8_t>(ptr[m - 1])];
    }

    return {};
}


// Предвычисление префиксов (оптимизировано под кэш)
inline void computePrefix(const char* pattern, size_t m, size_t* prefix) noexcept
{
    prefix[0] = 0;
    size_t k = 0;
    for (size_t i = 1; i < m; ++i)
    {
        while (k && pattern[k] != pattern[i])
        {
            k = prefix[k - 1];
        }
        k += (pattern[k] == pattern[i]);
        prefix[i] = k;
    }
}

// Оптимизированный KMP-поиск
inline StringPosition kmpFind(const char* text, size_t n, const char* pattern, size_t m) noexcept
{
    if (m == 0 || n < m)
    {
        return StringPosition();
    }

    // Если шаблон короткий — не выделяем память
    size_t smallPrefix[64];
    size_t* prefix = (m <= 64) ? smallPrefix : new size_t[m];

    computePrefix(pattern, m, prefix);

    size_t q = 0;
    const char* textEnd = text + n;

    for (const char* p = text; p != textEnd; ++p)
    {
        while (q && pattern[q] != *p)
        {
            q = prefix[q - 1];
        }
        if (pattern[q] == *p)
        {
            ++q;
            if (q == m)
            {
                if (prefix != smallPrefix)
                    delete[] prefix;
                return StringPosition(static_cast<size_t>(p - text - m + 1));
            }
        }
    }

    if (prefix != smallPrefix)
        delete[] prefix;

    return StringPosition();
}


// ���������� ������� ������� ��������� ��������� ��� StringPosition::None
inline StringPosition nativeFind(const char* text, size_t textLen, const char* pattern, size_t patternLen) noexcept
{
    if (patternLen == 0 || textLen < patternLen)
        return StringPosition{}; // None

    for (size_t i = 0; i <= textLen - patternLen; i++)
    {
        size_t j = 0;
        for (; j < patternLen; j++)
        {
            if (text[i + j] != pattern[j])
                break;
        }

        if (j == patternLen)
            return StringPosition{i}; // Нашли совпадение
    }

    return StringPosition{}; // None
}




#endif