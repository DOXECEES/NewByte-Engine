#ifndef SRC_UTILS_TIMER_HPP
#define SRC_UTILS_TIMER_HPP

#include <chrono>
namespace nb
{
    namespace Utils
    {
        class Timer
        {
        public:
            static void init() noexcept;
            static void reset() noexcept;

            static float timeElapsedSinceInit() noexcept;
            static float timeElapsed() noexcept;

        private:
            static inline std::chrono::time_point<std::chrono::high_resolution_clock> initTime  = std::chrono::high_resolution_clock::now();
            static inline std::chrono::time_point<std::chrono::high_resolution_clock> start     = std::chrono::high_resolution_clock::now();
        };
    }
};
#endif