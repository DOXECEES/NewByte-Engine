cmake_minimum_required(VERSION 3.25)
project(NewByte LANGUAGES CXX)

# ============================================================================  
# Настройки профайлера
# ============================================================================  

option(USE_VS_PROFILER "Enable VS Memory Profiler for memory tracking" ON)

# ============================================================================  
# Глобальные опции компилятора для MSVC
# ============================================================================  

if(MSVC)
    # Кодировка исходников UTF-8
    add_compile_options($<$<CXX_COMPILER_ID:MSVC>:/utf-8>)
    
    if(USE_VS_PROFILER)
        add_compile_definitions(_CRTDBG_MAP_ALLOC)
    endif()
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
# Функция для применения debug-флагов к таргету
# ============================================================================  

function(apply_debug_flags target)
    if(TARGET ${target} AND USE_VS_PROFILER)
        # Полная отладочная информация только для Debug
        target_compile_options(${target} PRIVATE
            $<$<CONFIG:Debug>:/Zi /Od>
        )
        target_link_options(${target} PRIVATE
            $<$<CONFIG:Debug>:/DEBUG>
        )
        target_compile_definitions(${target} PRIVATE
            $<$<CONFIG:Debug>:USE_VS_PROFILER>
        )
        # MSVC Runtime Debug DLL
        set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
        message(STATUS "VS Memory Profiler enabled for target: ${target}")
    endif()
endfunction()

# ============================================================================  
# Применяем флаги ко всем основным таргетам
# ============================================================================  

set(ALL_TARGETS NewByte-Engine SDK NewByte-UI-Lib Common nbstl tests)

#foreach(t ${ALL_TARGETS})
    #apply_debug_flags(${t})
#endforeach()

# ============================================================================  
# Code coverage (опционально)
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
# PVS-Studio integration
# ============================================================================  

find_program(PVS_ANALYZER NAMES pvs-studio-analyzer.exe)
find_program(PVS_CONVERTER NAMES plog-converter.exe)

if(PVS_ANALYZER AND PVS_CONVERTER)
    add_custom_target(pvs
        COMMAND ${PVS_ANALYZER} analyze --build ${CMAKE_BINARY_DIR} --output ${CMAKE_BINARY_DIR}/pvs.log
        COMMAND ${PVS_CONVERTER} -a GA:1,2,3,4 -t html -o ${CMAKE_BINARY_DIR}/pvs.html ${CMAKE_BINARY_DIR}/pvs.log
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running PVS-Studio analysis"
    )
endif()

# ============================================================================  
# Рекомендации по snapshot (ручной)
# ============================================================================  

# #ifdef USE_VS_PROFILER
# #include <crtdbg.h>
# _CrtDumpMemoryLeaks(); // выведет текущие утечки в Output Window
# #endif
