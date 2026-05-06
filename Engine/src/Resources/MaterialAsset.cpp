#include "MaterialAsset.hpp"

#include "Renderer/IRenderAPI.hpp"

namespace nb::Resource
{
    void MaterialAsset::bind()
    {
        m_shader->use();

        for (auto& [name, prop] : m_properties)
        {
            if (std::holds_alternative<float>(prop.value))
            {
                m_shader->setUniformFloat(name, std::get<float>(prop.value));
            }
            else if (std::holds_alternative<Color>(prop.value))
            {
                m_shader->setUniformVec4(name, std::get<Color>(prop.value).asVec4());
            }
            else if (std::holds_alternative<Ref<TextureAsset>>(prop.value))
            {
                auto texAsset    = std::get<Ref<TextureAsset>>(prop.value);
                auto internalTex = texAsset->getInternalTexture();

                uint64_t handle = internalTex->getHandle();

                m_shader->setUniformUint64(name, handle);
            }
        }
    }

}; 
