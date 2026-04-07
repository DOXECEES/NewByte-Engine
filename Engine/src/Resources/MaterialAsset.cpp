#include "MaterialAsset.hpp"

#include "Renderer/IRenderAPI.hpp"

namespace nb::Resource
{
    void MaterialAsset::bind()
    {
        m_shader->use();

        int textureSlot = 0;
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
                auto texAsset = std::get<Ref<TextureAsset>>(prop.value);
                //renderApi->bindTexture(textureSlot, texAsset->getInternalTexture()->getId());
                glActiveTexture(GL_TEXTURE0 + textureSlot);
                glBindTexture(GL_TEXTURE_2D, texAsset->getInternalTexture()->getId());
                m_shader->setUniformInt(name, textureSlot);
                textureSlot++;
            }
        }
    }

}; // namespace nb::Resource
