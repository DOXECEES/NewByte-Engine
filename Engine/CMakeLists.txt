cmake_minimum_required(VERSION 3.25.0)

project(NewByte_Engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(${PROJECT_NAME} WIN32 
        src/main.cpp

        src/Debug.cpp
        src/Debug.hpp
        src/Subsystems.cpp
        src/Subsystems.hpp
        src/Window.cpp
        src/Window.hpp

        src/UI/Button.cpp
        src/UI/Button.hpp

        src/Renderer/Renderer2D.hpp
        src/Renderer/Renderer2D.cpp
        src/Renderer/Renderer.hpp
        src/Renderer/Renderer.cpp        

        src/Math/Vector2.cpp
        src/Math/Vector2.hpp

        src/Math/Vector3.cpp
        src/Math/Vector3.hpp

        src/Math/Vector4.cpp
        src/Math/Vector4.hpp

        src/Loaders/BitmapLoader.cpp
        src/Loaders/BitmapLoader.hpp
        src/Loaders/PngLoader.cpp
        src/Loaders/PngLoader.hpp

        src/Templates/BinaryTree.hpp

        src/Shared/HuffmanTree.cpp
        src/Shared/HuffmanTree.hpp

        )


if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32)
endif()

