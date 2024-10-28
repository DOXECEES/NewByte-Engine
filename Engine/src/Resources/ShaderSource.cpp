#include "ShaderSource.hpp"

nb::Resource::ShaderSource::ShaderSource(const std::string &str) noexcept
    :source(std::move(str))
{
}

//

// shaderSource  == char*, 
// textureSource == char*,

// IResource -> ShaderSource -> IndependShader -> OpenGLShader      (engine setting)
//                                             -> DirectXShader 

// IResource -> TextureSource -> IndependTexture -> OpenGLTexture   (engine setting)
//                                               -> DirectXTexture

// IResource -> JsonSource -> Json possible YAML or XML;              ()

// auto tex = ResMan->loadResource<IndependTexture>("./assets/Texture/tree.png");
// tex->attach();

//