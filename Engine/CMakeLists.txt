cmake_minimum_required(VERSION 3.25.0)

project(NewByte_Engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(-DCMAKE_GENERATOR_PLATFORM x64)

add_subdirectory(dependencies/glad)



add_library(${PROJECT_NAME} STATIC 
        src/main.cpp
        
        src/Fatal.cpp
        src/Fatal.hpp

        src/Input/Input.cpp
        src/Input/Input.hpp
        src/Input/Keyboard.cpp
        src/Input/Keyboard.hpp
        src/Input/Mouse.cpp
        src/Input/Mouse.hpp

        src/Debug.cpp
        src/Debug.hpp
        src/Subsystems.cpp
        src/Subsystems.hpp
        src/Window.cpp
        src/Window.hpp

        src/Core/Engine.cpp
        src/Core/Engine.hpp
        src/Core/EngineSettings.cpp
        src/Core/EngineSettings.hpp

        src/UI/Button.cpp
        src/UI/Button.hpp

        src/Manager/ResourceManager.cpp
        src/Manager/ResourceManager.hpp

        src/Resources/IResource.cpp
        src/Resources/IResource.hpp
        src/Resources/ShaderSource.cpp
        src/Resources/ShaderSource.hpp

        src/Renderer/Renderer2D.hpp
        src/Renderer/Renderer2D.cpp
        src/Renderer/Renderer.hpp
        src/Renderer/Renderer.cpp
        src/Renderer/IRenderAPI.hpp
        src/Renderer/IRenderAPI.cpp     
        src/Renderer/Shader.cpp
        src/Renderer/Shader.hpp
        src/Renderer/SceneGraph.cpp
        src/Renderer/SceneGraph.hpp
        src/Renderer/Camera.cpp
        src/Renderer/Camera.hpp
        src/Renderer/Mesh.cpp
        src/Renderer/Mesh.hpp
        src/Renderer/Skybox.cpp
        src/Renderer/Skybox.hpp
        src/Renderer/Texture.cpp
        src/Renderer/Texture.hpp
        src/Renderer/Light.hpp
        src/Renderer/Light.cpp

        src/Renderer/RendererStructures.hpp

        src/Renderer/Objects/Objects.cpp
        src/Renderer/Objects/Objects.hpp

        src/Renderer/OpenGL/OpenGLRender.cpp        
        src/Renderer/OpenGL/OpenGLRender.hpp        
        src/Renderer/OpenGL/IBuffer.cpp
        src/Renderer/OpenGL/IBuffer.hpp
        src/Renderer/OpenGL/VBO.cpp
        src/Renderer/OpenGL/VBO.hpp
        src/Renderer/OpenGL/EBO.cpp
        src/Renderer/OpenGL/EBO.hpp
        src/Renderer/OpenGL/VertexArray.cpp
        src/Renderer/OpenGL/VertexArray.hpp
        src/Renderer/OpenGL/UBO.cpp
        src/Renderer/OpenGL/UBO.hpp
        src/Renderer/OpenGL/OpenGlShader.cpp
        src/Renderer/OpenGL/OpenGlShader.hpp
        src/Renderer/OpenGL/FBO.cpp
        src/Renderer/OpenGL/FBO.hpp
        src/Renderer/OpenGL/OpenGLTexture.cpp
        src/Renderer/OpenGL/OpenGLTexture.hpp



        

        
        src/Math/Constants.hpp
        src/Math/Vector2.cpp
        src/Math/Vector2.hpp
        src/Math/Vector3.cpp
        src/Math/Vector3.hpp
        src/Math/Vector4.cpp
        src/Math/Vector4.hpp
        src/Math/AABB.cpp
        src/Math/AABB.hpp
        src/Math/AABB3D.cpp
        src/Math/AABB3D.hpp
        src/Math/Quaternion.cpp
        src/Math/Quaternion.hpp


        src/Math/Matrix/Matrix.cpp
        src/Math/Matrix/Matrix.hpp
        src/Math/Matrix/Transformation.cpp
        src/Math/Matrix/Transformation.hpp

        src/Math/RayCast/RayPicker.cpp
        src/Math/RayCast/RayPicker.hpp

        src/Loaders/Factory/IFactoryLoader.cpp
        src/Loaders/Factory/IFactoryLoader.hpp
        src/Loaders/Factory/ShaderFactory.cpp
        src/Loaders/Factory/ShaderFactory.hpp
        src/Loaders/Factory/ObjFactory.cpp
        src/Loaders/Factory/ObjFactory.hpp

        src/Loaders/BitmapLoader.cpp
        src/Loaders/BitmapLoader.hpp
        src/Loaders/PngLoader.cpp
        src/Loaders/PngLoader.hpp
        src/Loaders/ShaderLoader.cpp
        src/Loaders/ShaderLoader.hpp

        src/Loaders/ObjLoader.cpp
        src/Loaders/ObjLoader.hpp

        src/Loaders/JSON/Json.cpp
        src/Loaders/JSON/Json.hpp
        src/Loaders/JSON/JsonLexer.cpp
        src/Loaders/JSON/JsonLexer.hpp
        src/Loaders/JSON/JsonParser.cpp
        src/Loaders/JSON/JsonParser.hpp
        src/Loaders/JSON/JsonWriter.cpp
        src/Loaders/JSON/JsonWriter.hpp
        src/Loaders/JSON/Node.cpp
        src/Loaders/JSON/Node.hpp

        src/Templates/BinaryTree.hpp

        src/Shared/HuffmanTree.cpp
        src/Shared/HuffmanTree.hpp

        src/Utils/BitReader.cpp
        src/Utils/BitReader.hpp
        src/Utils/DeflateDecoder.cpp
        src/Utils/DeflateDecoder.hpp

        dependencies/lodepng/lodepng.cpp


        )


find_package(OpenGL REQUIRED)

target_include_directories(${PROJECT_NAME} 
    PUBLIC OpenGL:GL
    PUBLIC dependencies/glad/include
)
        
        
        
target_link_directories(${PROJECT_NAME} 
    PRIVATE dependencies/glad/src
)
        
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32 opengl32 glad)
endif()

