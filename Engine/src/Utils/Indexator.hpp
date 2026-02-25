#ifndef SRC_UTILS_DEFLATEDECODER_HPP
#define SRC_UTILS_DEFLATEDECODER_HPP

#include <Windows.h>
#include "../Core.hpp"

#include <stack>
#include <limits>
#include <string>
#undef min
#undef max


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


        inline std::wstring toWString(const std::string& str) noexcept
        {
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);

            // Convert to wstring
            std::wstring wstrTo(size_needed, 0);
            MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

            return wstrTo;

        }


    };
};

#endif