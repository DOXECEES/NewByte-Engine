#ifndef SRC_UTILS_DEFLATEDECODER_HPP
#define SRC_UTILS_DEFLATEDECODER_HPP

#include "../Core.hpp"

#include <stack>
#include <limits>
namespace nb
{
    namespace Utils
    {
        class Indexator
        {
        public:
            explicit Indexator() = default;
            explicit Indexator(const int min, const int max);

            int next() const noexcept;
            NB_NODISCARD int index() noexcept;
            void freeIndex(const int index) noexcept;

        private:
            std::stack<int> freeIndexes;

            int             currentIndex    = 0;
            int             min             = 0;
            int max; //= std::numeric_limits<int>::max();
        };
    };
};

#endif