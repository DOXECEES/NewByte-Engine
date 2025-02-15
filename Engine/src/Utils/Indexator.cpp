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

        NB_NODISCARD int Indexator::index() noexcept
        {
            if(freeIndexes.empty())
                return currentIndex++;

            return freeIndexes.top();
        }

        void Indexator::freeIndex(const int index) noexcept
        {
            if(currentIndex <= index)
            return;
        
            freeIndexes.push(index);   
        }

    };
};