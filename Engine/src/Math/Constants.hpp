#ifndef SRC_MATH_CONTANTS_HPP
#define SRC_MATH_CONTANTS_HPP

#include <cmath>

namespace nb
{
    namespace Math
    {
        class Constants
        {
        
        public:

            static constexpr size_t PRECISION = 10;
            static constexpr double PI = 3.141592;
            inline static constexpr float HALF_OF_CIRCLE_IN_DEG = 180.0f;

        private:

            static constexpr double calculatePi(const size_t precision)
            {
                double res = 0;

                for (int i = 0; i < precision; i++)
                {
                    double temp = 1 / std::pow(16, i);
                    temp *= ((4.0 / (8.0 * i + 1.0)) - (2.0 / (8.0 * i + 4)) - (1.0 / (8.0 * i + 5)) - (1.0 / (8.0 * i + 6)));
                    res += temp;
                }

                return res;
            }
        };
    };
};

#endif

