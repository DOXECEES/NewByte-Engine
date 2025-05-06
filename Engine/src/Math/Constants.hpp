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

            static constexpr size_t         PRECISION               = 25;
            static constexpr double         PI                      = 3.141592653589793238462643383279502884;
            inline static constexpr float   HALF_OF_CIRCLE_IN_DEG   = 180.0;

        //private:

            static constexpr double calculatePi()
            {
                double res = 0;

                for (int i = 0; i < PRECISION; i++)
                {
                    double p = 16;
                    for (int j = 0; j < i; j++)
                    {
                        p *= 16;
                    }
                        double temp = 1/ p;
                    temp *= ((4.0 / (8.0 * i + 1.0)) - (2.0 / (8.0 * i + 4)) - (1.0 / (8.0 * i + 5)) - (1.0 / (8.0 * i + 6)));
                    res += temp;
                }

                return res;
            }
        };
    };
};

#endif

