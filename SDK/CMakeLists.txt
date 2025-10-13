cmake_minimum_required(VERSION 3.25.0)

project(NewByte_SDK)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)



add_executable(${PROJECT_NAME} WIN32 

    src/main.cpp

    src/SceneWindow.cpp
    src/SceneWindow.hpp
    src/HierarchyWindow.cpp
    src/HierarchyWindow.hpp
    src/PropertiesWindow.cpp
    src/PropertiesWindow.hpp

    src/Ui/Slider.cpp
    src/Ui/Slider.hpp
    src/Ui/ComboBox.hpp
    src/Ui/ComboBox.cpp
    src/Ui/RadioGroup.cpp
    src/Ui/RadioGroup.hpp
    src/Ui/Button.cpp
    src/Ui/Button.hpp
    src/Ui/IWidget.cpp
    src/Ui/IWidget.hpp
    src/Ui/UiMapper.cpp
    src/Ui/UiMapper.hpp

    src/UiStore.cpp
    src/UiStore.hpp
)

set(ASSETS_SRC_DIR "${CMAKE_SOURCE_DIR}/Assets")
if(NOT EXISTS "${ASSETS_SRC_DIR}")
    set(ASSETS_SRC_DIR "${CMAKE_SOURCE_DIR}/assets")
endif()

if(NOT EXISTS "${ASSETS_SRC_DIR}")
    message(WARNING "Assets directory not found (checked ${CMAKE_SOURCE_DIR}/Assets and ${CMAKE_SOURCE_DIR}/assets). No copy will be performed.")
else()
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_SRC_DIR}" "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets"
        COMMENT "Copying Assets folder to $<TARGET_FILE_DIR:${PROJECT_NAME}>/Assets"
    )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
    ../Engine/src
    ../NewByte-UI-Lib/src
)

target_link_directories(${PROJECT_NAME} PUBLIC
    C:/rep/Hex/NewByte-Engine/build/Engine/Debug
    C:/rep/Hex/NewByte-UI-Lib/build/NewByte-UI-Lib/Debug
)



if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32 NewByte_Engine NewByte_UI_Lib)
endif()
