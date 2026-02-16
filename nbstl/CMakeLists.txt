cmake_minimum_required(VERSION 3.25.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 1. Создаём header-only библиотеку
add_library(nbstl INTERFACE)

# 2. Добавляем include пути
target_include_directories(nbstl INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src        # свои .hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Common  # NbCore.hpp
)

# 3. Линкуем NbCommon (header-only)
target_link_libraries(nbstl INTERFACE NbCommon)

# 4. Настройки компиляции (опционально)
target_compile_options(nbstl INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive-> 
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

if(WIN32)
    target_compile_definitions(nbstl INTERFACE UNICODE _UNICODE)
endif()
