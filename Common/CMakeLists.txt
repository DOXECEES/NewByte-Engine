cmake_minimum_required(VERSION 3.16)

project(NbCommon LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(NbCommon STATIC)

target_compile_features(NbCommon PUBLIC cxx_std_20)

target_include_directories(NbCommon
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(NbCommon INTERFACE nbstl)


target_sources(NbCommon
    PRIVATE
        NbCore.hpp
        Color.cpp
        Color.hpp
        NbRect.hpp
)