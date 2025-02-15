#ifndef SRC_UTILS_DEFLATEDECODER_HPP
#define SRC_UTILS_DEFLATEDECODER_HPP

#include "../Core.hpp"

#include <stack>

namespace nb
{
    namespace Utils
    {
        class Indexator
        {
        public:
            explicit Indexator(const int min, const int max);

            NB_NODISCARD int index() noexcept;
            void freeIndex(const int index) noexcept;

        private:
            std::stack<int> freeIndexes;

            int currentIndex;
            int min;
            int max;    
        };
    };
};

#endif