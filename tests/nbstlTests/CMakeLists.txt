cmake_minimum_required(VERSION 3.25)

project(nbstlTests)

include(FetchContent)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.9.0
)

FetchContent_MakeAvailable(benchmark)



add_executable(nbstlTests 
    BitManipulationTest.cpp 
    StringTest.cpp
)

# ������� GoogleTest
target_link_libraries(nbstlTests PRIVATE gtest_main
        benchmark::benchmark
        benchmark::benchmark_main
)

# ���������� include-����: nbstl + Common
target_include_directories(nbstlTests PRIVATE 
    ${CMAKE_SOURCE_DIR}/nbstl/src
    ${CMAKE_SOURCE_DIR}/Common
)


# ����-�������� ������
include(GoogleTest)
