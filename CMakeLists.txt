cmake_minimum_required(VERSION 3.25)
project(NewByte LANGUAGES CXX)

# ============================================================================  
# ��������� ���������� � ������ ������ ������
# ============================================================================  

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 2. Указываем глобальный стандарт C++20 (или C++23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ������������� ����������� ���������� CRT (������ ��������� � Output Visual Studio)
option(USE_VS_PROFILER "Enable VS Memory Profiler for memory tracking" ON)

# ������������� �������������� ����������� AddressSanitizer (ASan) ��� ������ ������
option(USE_ASAN "Enable Address Sanitizer (ASan)" OFF)

# ============================================================================  
# ������������� �������
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
# ������� ��� ���������� �������� (UTF-8, �������, ��������� � ASan)
# ============================================================================  

function(apply_project_settings target)
    if(NOT TARGET ${target})
        return()
    endif()

    # ��������� ��� �������. ��� INTERFACE ��������� ���������� ����� INTERFACE,
    # ��� ������� (static, shared, exe) � PRIVATE.
    get_target_property(target_type ${target} TYPE)
    if (target_type STREQUAL "INTERFACE_LIBRARY")
        set(scope INTERFACE)
    else()
        set(scope PRIVATE)
    endif()

    # 1. ����������� �������� ����� (������ ��� MSVC)
    if(MSVC)
        target_compile_options(${target} ${scope} /utf-8)
    endif()

    # 2. ��������� ���������� � ��������� ���������� �������� (����������� Unresolved allocations)
    if(USE_VS_PROFILER)
        # ��� INTERFACE ��������� ������������ ������� ���, ��� �� ����������
        target_compile_definitions(${target} ${scope}
            $<$<CONFIG:Debug>:USE_VS_PROFILER>
            $<$<CONFIG:Debug>:_CRTDBG_MAP_ALLOC>
        )

        # ����� ���������� � �������� (������ ��� ������������� ��������)
        if (NOT target_type STREQUAL "INTERFACE_LIBRARY")
            # /Zi � /DEBUG ���������� ������ PDB-����� ��� ���������� ����-������� � ����������
            target_compile_options(${target} PRIVATE $<$<CONFIG:Debug>:/Zi /Od>)
            target_link_options(${target} PRIVATE $<$<CONFIG:Debug>:/DEBUG>)
            
            set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
        endif()
    endif()

    # 3. ��������� Address Sanitizer (ASan)
    if(USE_ASAN)
        if(MSVC)
            # ��������� ASan ��� ����������� MSVC
            target_compile_options(${target} ${scope} /fsanitize=address)
            if (NOT target_type STREQUAL "INTERFACE_LIBRARY")
                # ASan ����������� � ������������ ���������
                target_link_options(${target} PRIVATE /INCREMENTAL:NO)
            endif()
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            # ��������� ASan ��� GCC / Clang
            target_compile_options(${target} ${scope} -fsanitize=address -fno-omit-frame-pointer)
            if (NOT target_type STREQUAL "INTERFACE_LIBRARY")
                target_link_options(${target} PRIVATE -fsanitize=address)
            endif()
        endif()
    endif()
endfunction()

# ============================================================================  
# ��������� ��������� �� ���� �������� �������
# ============================================================================  

# ��������: ������ ���� ������ ����� ��������� � ������� ��������,
# ���������� ������ ������� add_library() � add_executable().
# �������� ��� "SDK" �� "NewByte_SDK" �������� ����� ������������ ����� �� ��������������.
set(ALL_TARGETS 
    NewByte-Engine 
    NewByte_SDK 
    NewByte-UI-Lib 
    Common 
    nbstl 
    tests
)

foreach(t ${ALL_TARGETS})
    apply_project_settings(${t})
endforeach()

# ============================================================================  
# PVS-Studio � ���������
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