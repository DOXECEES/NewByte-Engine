// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Indexator.hpp"


namespace nb
{
    namespace Utils
    {
        Indexator::Indexator(const int min, const int max)
            : min(min)
            , max(max)
            , currentIndex(min)
        {}

        int Indexator::next() const noexcept
        {
            if (freeIndexes.empty())
            {
                return currentIndex;
            }

            return freeIndexes.top();
        }

        NB_NODISCARD int Indexator::index() noexcept
        {
            if (!freeIndexes.empty())
            {
                int idx = freeIndexes.top();
                freeIndexes.pop();
                return idx;
            }

            if (currentIndex > max)
            {
                return -1; 
            }

            return currentIndex++;
        }

        void Indexator::freeIndex(const int index) noexcept
        {
            if (index < min || index > max)
            {
                return;
            }

            if (index >= currentIndex)
            {
                return;
            }

            freeIndexes.push(index);
        }

    };
};