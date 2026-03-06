cmake_minimum_required(VERSION 3.16)

project(NbCommon LANGUAGES CXX)

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
)