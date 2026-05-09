#include "MaterialAsset.hpp"

#include "Renderer/IRenderAPI.hpp"

namespace nb::Resource
{
    void MaterialAsset::bind(Ref<Renderer::Shader> sh)
    {
        sh->use();

        for (auto& [name, prop] : m_properties)
        {
            if (std::holds_alternative<float>(prop.value))
            {
                sh->setUniformFloat(name, std::get<float>(prop.value));
            }
            else if (std::holds_alternative<Color>(prop.value))
            {
                sh->setUniformVec4(name, std::get<Color>(prop.value).asVec4());
            }
            else if (std::holds_alternative<Ref<TextureAsset>>(prop.value))
            {
                auto texAsset    = std::get<Ref<TextureAsset>>(prop.value);
                auto internalTex = texAsset->getInternalTexture();

                uint64_t handle = internalTex->getHandle();

                sh->setUniformUint64(name, handle);
            }
        }
    }

}; 
