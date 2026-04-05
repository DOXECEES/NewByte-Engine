cmake_minimum_required(VERSION 3.25.0)

project(NewByte_Engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(DEPENDENCIES_DIR "${PROJECT_SOURCE_DIR}/dependencies")
set(LUA_DIR "${DEPENDENCIES_DIR}/lua")
set(SOL2_DIR "${DEPENDENCIES_DIR}/sol")
set(TINY_GIZMO_DIR "${DEPENDENCIES_DIR}/tinyGizmo")

add_library(
    lua STATIC
    ${LUA_DIR}/lapi.c
    ${LUA_DIR}/lauxlib.c
    ${LUA_DIR}/lbaselib.c
    ${LUA_DIR}/lcode.c
    ${LUA_DIR}/lcorolib.c
    ${LUA_DIR}/lctype.c
    ${LUA_DIR}/ldblib.c
    ${LUA_DIR}/ldebug.c
    ${LUA_DIR}/ldo.c
    ${LUA_DIR}/ldump.c
    ${LUA_DIR}/lfunc.c
    ${LUA_DIR}/lgc.c
    ${LUA_DIR}/linit.c
    ${LUA_DIR}/liolib.c
    ${LUA_DIR}/llex.c
    ${LUA_DIR}/lmathlib.c
    ${LUA_DIR}/lmem.c
    ${LUA_DIR}/loadlib.c
    ${LUA_DIR}/lobject.c
    ${LUA_DIR}/lopcodes.c
    ${LUA_DIR}/loslib.c
    ${LUA_DIR}/lparser.c
    ${LUA_DIR}/lstate.c
    ${LUA_DIR}/lstring.c
    ${LUA_DIR}/lstrlib.c
    ${LUA_DIR}/ltable.c
    ${LUA_DIR}/ltablib.c
    ${LUA_DIR}/ltm.c

    ${LUA_DIR}/lundump.c
    ${LUA_DIR}/lutf8lib.c
    ${LUA_DIR}/lvm.c
    ${LUA_DIR}/lzio.c
)

add_library(sol2 INTERFACE)
target_include_directories(sol2 INTERFACE ${SOL2_DIR})

add_library(tinyGizmo STATIC
    ${TINY_GIZMO_DIR}/tiny-gizmo.cpp 
    ${TINY_GIZMO_DIR}/tiny-gizmo.hpp
)
target_include_directories(tinyGizmo PUBLIC ${TINY_GIZMO_DIR})


set(-DCMAKE_GENERATOR_PLATFORM x64)

add_subdirectory(dependencies/glad)

file(GLOB RESOURCE_FILES 
    "res/*"
    "shaders/*")

set (SRC 
        src/main.cpp
        
        src/Common/StringUtils.cpp
        src/Common/StringUtils.hpp

        src/Fatal.cpp
        src/Fatal.hpp

        src/BVH.hpp

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
        src/Core/ShaderSystem.cpp
        src/Core/ShaderSystem.hpp

        src/UI/Button.cpp
        src/UI/Button.hpp

        src/Manager/ResourceManager.cpp
        src/Manager/ResourceManager.hpp

        src/Resources/IResource.cpp
        src/Resources/IResource.hpp
        src/Resources/ShaderSource.cpp
        src/Resources/ShaderSource.hpp

        src/Resources/TextureAsset.hpp

        src/Resources/MaterialAsset.cpp
        src/Resources/MaterialAsset.hpp
        src/Resources/IhdrResource.hpp

        src/Serialize/ISerializable.cpp
        src/Serialize/ISerializable.hpp
        src/Serialize/IArchive.cpp
        src/Serialize/IArchive.hpp
        src/Serialize/JsonArchive.cpp
        src/Serialize/JsonArchive.hpp


        src/Renderer/Renderer2D.hpp
        src/Renderer/Renderer2D.cpp
        src/Renderer/Renderer.hpp
        src/Renderer/Renderer.cpp
        src/Renderer/IRenderAPI.hpp
        src/Renderer/IRenderAPI.cpp     
        src/Renderer/Shader.cpp
        src/Renderer/Shader.hpp
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
        src/Renderer/OpenGL/ShaderConstants.hpp
        src/Renderer/OpenGL/DepthBuffer.cpp
        src/Renderer/OpenGL/DepthBuffer.hpp
        src/Renderer/OpenGL/ShadowMapFbo.cpp
        src/Renderer/OpenGL/ShadowMapFbo.hpp
        src/Renderer/OpenGL/PostProcessFbo.cpp
        src/Renderer/OpenGL/PostProcessFbo.hpp
        src/Renderer/OpenGL/OpenGLCubemap.cpp
        src/Renderer/OpenGL/OpenGLCubemap.hpp
        
        src/Renderer/PipelineCache.cpp
        src/Renderer/PipelineCache.hpp
        src/Renderer/IFrameBuffer.hpp
        src/Renderer/Material.cpp
        src/Renderer/Material.hpp
        src/Renderer/ContextMeshCache.cpp
        src/Renderer/ContextMeshCache.hpp
        src/Renderer/Scene.cpp
        src/Renderer/Scene.hpp
        src/Renderer/Cubemap.cpp
        src/Renderer/Cubemap.hpp

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
        src/Math/Plane.hpp


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
        src/Loaders/Factory/TextureFactory.cpp
        src/Loaders/Factory/TextureFactory.hpp
        src/Loaders/Factory/MaterialFactory.cpp
        src/Loaders/Factory/MaterialFactory.hpp
        src/Loaders/Factory/IhdrFactory.cpp
        src/Loaders/Factory/IhdrFactory.hpp

        src/Loaders/BitmapLoader.cpp
        src/Loaders/BitmapLoader.hpp
        src/Loaders/PngLoader.cpp
        src/Loaders/PngLoader.hpp
        src/Loaders/ShaderLoader.cpp
        src/Loaders/ShaderLoader.hpp
        src/Loaders/TextureLoader.cpp
        src/Loaders/TextureLoader.hpp
        src/Loaders/MaterialLoader.cpp
        src/Loaders/MaterialLoader.hpp
        src/Loaders/IhdrLoader.cpp
        src/Loaders/IhdrLoader.hpp
      

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
        src/Templates/NTree.hpp

        src/Shared/HuffmanTree.cpp
        src/Shared/HuffmanTree.hpp

        src/Utils/BitReader.cpp
        src/Utils/BitReader.hpp
        src/Utils/DeflateDecoder.cpp
        src/Utils/DeflateDecoder.hpp
        src/Utils/Timer.cpp
        src/Utils/Timer.hpp
        src/Utils/Indexator.cpp
        src/Utils/Indexator.hpp

        src/Error/ErrorManager.cpp
        src/Error/ErrorManager.hpp
        src/Error/IErrorPrinter.cpp
        src/Error/IErrorPrinter.hpp
        src/Error/ErrorConsolePrinter.cpp
        src/Error/ErrorConsolePrinter.hpp

        src/ECS/Ecs.hpp
        src/ECS/Ecs.cpp

        src/Scripting/ScriptComponent.cpp
        src/Scripting/ScriptComponent.hpp
        src/Scripting/ScriptEngine.cpp
        src/Scripting/ScriptEngine.hpp

        src/Physics/Physics.hpp

        src/Physics/TerrainCollider.cpp
        src/Physics/TerrainCollider.hpp

        dependencies/lodepng/lodepng.cpp
        dependencies/stb/stb_image.h

        src/MemoryTracker.cpp
        src/MemoryTracker.hpp

)

add_library(${PROJECT_NAME} STATIC ${SRC})


find_package(OpenGL REQUIRED)

target_include_directories(${PROJECT_NAME} 
    PUBLIC src
    PUBLIC OpenGL:GL
    PUBLIC dependencies/glad/include
    ../nbstl/src
    ../Common
    PUBLIC ${SOL2_DIR}    
    PUBLIC ${LUA_DIR}
    PUBLIC ${TINY_GIZMO_DIR}
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)
        
        
target_link_directories(${PROJECT_NAME} 
    PRIVATE dependencies/glad/src
)
        
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE UNICODE _UNICODE)
    target_link_libraries(${PROJECT_NAME} PRIVATE user32 opengl32 glad lua sol2 tinyGizmo)
    
endif()

foreach(source IN LISTS SRC)
  get_filename_component(source_path "${source}" PATH)
  string(REPLACE "/" "\\" source_path_msvc "${source_path}")
  source_group("${source_path_msvc}" FILES "${source}")
endforeach()
