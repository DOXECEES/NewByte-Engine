#include "Timer.hpp"

namespace nb
{
    namespace Utils
    {
        void Timer::init() noexcept
        {
            reset();
        }

        void Timer::reset() noexcept
        {
            initTime = std::chrono::high_resolution_clock::now();
            start = std::chrono::high_resolution_clock::now();
        }

        float Timer::timeElapsedSinceInit() noexcept
        {
            return std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - initTime).count();
        }

        float Timer::timeElapsed() noexcept
        {
            std::chrono::time_point<std::chrono::high_resolution_clock> elapse = start;
            start = std::chrono::high_resolution_clock::now();
            return std::chrono::duration<float>(start - elapse).count();
        }
    };
};