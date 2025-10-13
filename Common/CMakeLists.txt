cmake_minimum_required(VERSION 3.16)

project(NbCommon LANGUAGES CXX)

# Создаём header-only библиотеку
add_library(NbCommon INTERFACE)

# Задаём стандарт C++ и гарантируем поддержку
target_compile_features(NbCommon INTERFACE cxx_std_20)

# Указываем путь к заголовкам
target_include_directories(NbCommon
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)
