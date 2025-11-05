cmake_minimum_required(VERSION 3.16)
project(NbCommon LANGUAGES CXX)

# Создаём только если таргет ещё не существует
if(NOT TARGET NbCommon)
    add_library(NbCommon INTERFACE)

    target_compile_features(NbCommon INTERFACE cxx_std_20)

    target_include_directories(NbCommon
        INTERFACE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_sources(NbCommon INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/NbCore.hpp
    )
endif()
