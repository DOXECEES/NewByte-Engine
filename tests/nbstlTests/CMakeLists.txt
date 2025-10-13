cmake_minimum_required(VERSION 3.25)


project(nbstlTests)

add_executable(nbstlTests BitManipulationTest.cpp)


target_link_libraries(nbstlTests PRIVATE gtest_main)

target_include_directories(nbstlTests PRIVATE ${CMAKE_SOURCE_DIR}/nbstl/src)

gtest_discover_tests(nbstlTests)
