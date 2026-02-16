#ifndef NBSTL_SRC_HASH_HPP
#define NBSTL_SRC_HASH_HPP

#include "Types.hpp"
#include "BitManipulation.hpp"
#include "Array.hpp"

#include <cstring>

namespace nbstl
{
    inline uint32 read32(const void* p) noexcept
    {
        uint32 v;
        memcpy(&v, p, sizeof(v));
        return v;
    }

    inline uint32 xxHash32(const void* input, Size length, uint32 seed = 0) noexcept
    {
        static const uint32 PRIME32_1 = 2654435761U;
        static const uint32 PRIME32_2 = 2246822519U;
        static const uint32 PRIME32_3 = 3266489917U;
        static const uint32 PRIME32_4 = 668265263U;
        static const uint32 PRIME32_5 = 374761393U;

        const uint8_t* p = static_cast<const uint8_t*>(input);
        const uint8_t* const bEnd = p + length;
        uint32 h32;

        if (length >= 16)
        {
            const uint8_t* const limit = bEnd - 16;
            Array<uint32, 4> accumulator =
            {
                seed + PRIME32_1 + PRIME32_2,
                seed + PRIME32_2,
                seed + 0,
                seed - PRIME32_1,
            };
            do
            {
                for (int i = 0; i < 4; i++)
                {
                    accumulator[i] += read32(p) * PRIME32_2;
                    accumulator[i] = rotl(accumulator[i], 13);
                    accumulator[i] *= PRIME32_1;
                    p += 4;
                }

            } while (p <= limit);

            h32 = rotl(accumulator[0], 1)
                + rotl(accumulator[1], 7)
                + rotl(accumulator[2], 12)
                + rotl(accumulator[3], 18);
        }
        else 
        {
            h32 = seed + PRIME32_5;
        }

        h32 += static_cast<uint32>(length);

        while (p + 4 <= bEnd)
        {
            h32 += (*reinterpret_cast<const uint32*>(p)) * PRIME32_3;
            h32 = rotl(h32, 17) * PRIME32_4;
            p += 4;
        }

        while (p < bEnd)
        {
            h32 += (*p) * PRIME32_5;
            h32 = rotl(h32, 11) * PRIME32_1;
            p++;
        }

        h32 ^= h32 >> 15;
        h32 *= PRIME32_2;
        h32 ^= h32 >> 13;
        h32 *= PRIME32_3;
        h32 ^= h32 >> 16;

        return h32;
    }
};

#endif 