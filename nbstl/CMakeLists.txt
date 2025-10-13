cmake_minimum_required(VERSION 3.25.0)

project(nbstl)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)



set(-DCMAKE_GENERATOR_PLATFORM x64)


set (SRC 
    src/NTree.hpp
    src/Alghorithm.hpp
    src/Uuid.hpp
    src/BitManipulation.hpp
)

add_subdirectory(../Common NbCommon_build)
target_link_libraries(nbstl PUBLIC NbCommon)

add_library(${PROJECT_NAME} STATIC ${SRC})



target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)        
        
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
endif()

foreach(source IN LISTS SRC)
  get_filename_component(source_path "${source}" PATH)
  string(REPLACE "/" "\\" source_path_msvc "${source_path}")
  source_group("${source_path_msvc}" FILES "${source}")
  
endforeach()
