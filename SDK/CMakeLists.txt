cmake_minimum_required(VERSION 3.25.0)

project(NewByte_SDK)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(MY_STACK_SIZE 4194304)


add_executable(${PROJECT_NAME} WIN32 

    src/main.cpp

    src/ImportWindow.cpp
    src/ImportWindow.hpp
    src/App.cpp
    src/App.hpp
    src/SceneModel.hpp

    src/TextureEditor.cpp
    src/TextureEditor.hpp

    src/AssetManger.cpp
    src/AssetManger.hpp

    src/MaterialEditor.cpp
    src/MaterialEditor.hpp

    src/AssetThumbnailWidget.hpp
    src/AssetThumbnailWidget.cpp

    src/FileSystemModel.hpp

    src/ComponentBrowser.hpp
    src/PrimitiveCreationDialog.hpp
    
    src/FramebufferVisualization.cpp
    src/FramebufferVisualization.hpp

    src/CameraBookmark.cpp
    src/CameraBookmark.hpp
    src/CameraBookmarkWindow.cpp
    src/CameraBookmarkWindow.hpp
    src/Camera/CameraSettingsController.cpp
    src/Camera/CameraSettingsController.hpp
    src/Camera/CameraSettings.hpp

    src/EngineSettingsController.hpp

    src/Scene/SceneWindow.cpp
    src/Scene/SceneWindow.hpp
    src/Scene/SceneController.cpp
    src/Scene/SceneController.hpp
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

target_link_options(NewByte_SDK PRIVATE "/STACK:${MY_STACK_SIZE}")


if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32 NewByte_Engine NewByte_UI_Lib)
endif()
