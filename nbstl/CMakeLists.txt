cmake_minimum_required(VERSION 3.25.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 2. Указываем глобальный стандарт C++20 (или C++23)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. ������ header-only ����������
add_library(nbstl INTERFACE)

# 2. ��������� include ����
target_include_directories(nbstl INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src        # ���� .hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../Common  # NbCore.hpp
)

# 3. ������� NbCommon (header-only)
target_link_libraries(nbstl INTERFACE NbCommon)

# 4. ��������� ���������� (�����������)
target_compile_options(nbstl INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive-> 
    /analyze
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

if(WIN32)
    target_compile_definitions(nbstl INTERFACE UNICODE _UNICODE)
endif()
