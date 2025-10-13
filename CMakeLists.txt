cmake_minimum_required(VERSION 3.25.0)
project(NewByte)

# Опция для включения покрытия кода
option(BUILD_COVERAGE "Build with code coverage support" OFF)

if(BUILD_COVERAGE)
    message(STATUS "Code coverage enabled")
    
    # Всегда используем OpenCppCoverage для Windows
    find_program(OPENCPPCOVERAGE OpenCppCoverage)
    if(OPENCPPCOVERAGE)
        message(STATUS "OpenCppCoverage found: ${OPENCPPCOVERAGE}")
    else()
        message(WARNING "OpenCppCoverage not found. Please install it from GitHub")
    endif()
endif()

add_subdirectory(SDK)
add_subdirectory(Engine)
add_subdirectory(NewByte-UI-Lib)
add_subdirectory(googletest)

enable_testing()  
add_subdirectory(tests)

# Цели для покрытия кода
if(BUILD_COVERAGE AND OPENCPPCOVERAGE)
    # Цель для генерации отчета покрытия
    add_custom_target(coverage
        COMMAND ${OPENCPPCOVERAGE} 
            --sources ${CMAKE_SOURCE_DIR}/Engine/src
            --sources ${CMAKE_SOURCE_DIR}/SDK/src  
            --sources ${CMAKE_SOURCE_DIR}/NewByte-UI-Lib/src
            --modules ${CMAKE_BINARY_DIR}/tests/Debug/*.exe
            --export_type html:${CMAKE_BINARY_DIR}/coverage_report
            -- ${CMAKE_BINARY_DIR}/tests/Debug/hsv_tests.exe
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Running code coverage with OpenCppCoverage"
        DEPENDS hsv_tests
    )
    
    # Цель для быстрого запуска тестов с покрытием
    add_custom_target(coverage-quick
        COMMAND ${OPENCPPCOVERAGE} 
            --sources ${CMAKE_SOURCE_DIR}/Engine/src
            --modules ${CMAKE_BINARY_DIR}/tests/Debug/*.exe
            -- ${CMAKE_BINARY_DIR}/tests/Debug/hsv_tests.exe
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Quick coverage run (console output only)"
        DEPENDS hsv_tests
    )
endif()