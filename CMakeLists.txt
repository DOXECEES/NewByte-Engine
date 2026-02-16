cmake_minimum_required(VERSION 3.25.0)
project(NewByte)

# ============================================================================  
# VS Memory Profiler настройки  
# ============================================================================  

option(USE_VS_PROFILER "Enable VS Memory Profiler for memory tracking" ON)

# Макрос для включения CRT debug tracking
if(MSVC AND USE_VS_PROFILER)
    add_compile_definitions(_CRTDBG_MAP_ALLOC)
endif()

# ============================================================================  
# Поддиректории проекта  
# ============================================================================  

add_subdirectory(Common)
add_subdirectory(nbstl)
add_subdirectory(SDK)
add_subdirectory(Engine)
add_subdirectory(NewByte-UI-Lib)
add_subdirectory(googletest)
enable_testing()
add_subdirectory(tests)

# ============================================================================  
# Функции для применения флагов ко всем целям  
# ============================================================================  

function(apply_debug_flags target)
    if(TARGET ${target} AND USE_VS_PROFILER)
        # Полная отладочная информация
        target_compile_options(${target} PRIVATE /Zi /Od /MDd)
        target_link_options(${target} PRIVATE /DEBUG)
        target_compile_definitions(${target} PRIVATE USE_VS_PROFILER)
        message(STATUS "VS Memory Profiler enabled for target: ${target}")
    endif()
endfunction()

# ============================================================================  
# Применяем профайлер ко всем основным целям  
# ============================================================================  

set(ALL_TARGETS NewByte-Engine SDK NewByte-UI-Lib Common nbstl tests)

foreach(t ${ALL_TARGETS})
    apply_debug_flags(${t})
endforeach()

# ============================================================================  
# Code coverage (по желанию)  
# ============================================================================  

option(BUILD_COVERAGE "Build with code coverage support" OFF)

if(BUILD_COVERAGE)
    find_program(OPENCPPCOVERAGE OpenCppCoverage)
    if(OPENCPPCOVERAGE)
        message(STATUS "OpenCppCoverage found: ${OPENCPPCOVERAGE}")
    else()
        message(WARNING "OpenCppCoverage not found. Please install it from GitHub")
    endif()
endif()

# ============================================================================  
# Рекомендация для snapshot  
# ============================================================================  

# В коде можно добавить, если хочешь, ручной snapshot для Profiler:
# #ifdef USE_VS_PROFILER
# #include <crtdbg.h>
# _CrtDumpMemoryLeaks(); // выведет текущие утечки в Output Window
# #endif
