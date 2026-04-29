cmake_minimum_required(VERSION 3.25)
project(NewByte LANGUAGES CXX)

# ============================================================================  
# Настройки профайлера
# ============================================================================  

option(USE_VS_PROFILER "Enable VS Memory Profiler for memory tracking" ON)

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
# Функция для применения настроек (UTF-8 и отладка)
# ============================================================================  

function(apply_project_settings target)
    if(NOT TARGET ${target})
        return()
    endif()

    # Проверяем тип таргета. Для INTERFACE библиотек используем режим INTERFACE,
    # для обычных (static, shared, exe) — PRIVATE.
    get_target_property(target_type ${target} TYPE)
    if (target_type STREQUAL "INTERFACE_LIBRARY")
        set(scope INTERFACE)
    else()
        set(scope PRIVATE)
    endif()

    # 1. Исправление русского языка (только для MSVC)
    if(MSVC)
        target_compile_options(${target} ${scope} /utf-8)
    endif()

    # 2. Настройки профайлера
    if(USE_VS_PROFILER)
        # Для INTERFACE библиотек мы не можем задать настройки компиляции/линковки "внутри" них,
        # но мы можем пробросить их тем, кто их использует.
        
        target_compile_definitions(${target} ${scope}
            $<$<CONFIG:Debug>:USE_VS_PROFILER>
            $<$<CONFIG:Debug>:_CRTDBG_MAP_ALLOC>
        )

        # Опции компиляции и линковки (только для не-INTERFACE таргетов, 
        # так как INTERFACE таргеты сами не компилируются)
        if (NOT target_type STREQUAL "INTERFACE_LIBRARY")
            target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:/Zi /Od>)
            target_link_options(${target} PRIVATE $<$<CONFIG:Debug>:/DEBUG>)
            
            set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
        endif()
    endif()
endfunction()

# ============================================================================  
# Применяем настройки ко всем твоим таргетам
# ============================================================================  

# Важно: список имен должен точно совпадать с именами в add_library/add_executable
set(ALL_TARGETS 
    NewByte-Engine 
    SDK 
    NewByte-UI-Lib 
    Common 
    nbstl 
    tests
)

foreach(t ${ALL_TARGETS})
    apply_project_settings(${t})
endforeach()

# ============================================================================  
# PVS-Studio и остальное
# ============================================================================  

find_program(PVS_ANALYZER NAMES pvs-studio-analyzer.exe)
find_program(PVS_CONVERTER NAMES plog-converter.exe)

if(PVS_ANALYZER AND PVS_CONVERTER)
    if(NOT TARGET pvs)
        add_custom_target(pvs
            COMMAND ${PVS_ANALYZER} analyze --build ${CMAKE_BINARY_DIR} --output ${CMAKE_BINARY_DIR}/pvs.log
            COMMAND ${PVS_CONVERTER} -a GA:1,2,3,4 -t html -o ${CMAKE_BINARY_DIR}/pvs.html ${CMAKE_BINARY_DIR}/pvs.log
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMENT "Running PVS-Studio analysis"
        )
    endif()
endif()