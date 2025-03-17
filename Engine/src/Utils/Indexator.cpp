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
            if(freeIndexes.empty())
                return currentIndex + 1;

            return freeIndexes.top();
        }

        NB_NODISCARD int Indexator::index() noexcept
        {
            if(freeIndexes.empty())
                return currentIndex++;

            int next = freeIndexes.top();
            freeIndexes.pop();
            return next;
        }

        void Indexator::freeIndex(const int index) noexcept
        {
            if(currentIndex <= index)
                return;
        
            freeIndexes.push(index);   
        }

    };
};