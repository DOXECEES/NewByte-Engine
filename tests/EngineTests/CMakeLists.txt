cmake_minimum_required(VERSION 3.25)


project(EngineTests)

add_executable(EngineTests ColorTest.cpp)


target_link_libraries(EngineTests PRIVATE NewByte_Engine gtest_main)

target_include_directories(EngineTests PRIVATE ${CMAKE_SOURCE_DIR}/Engine)

gtest_discover_tests(EngineTests)
