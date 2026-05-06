#ifndef SRC_RENDERER_SSAO_HPP
#define SRC_RENDERER_SSAO_HPP

#include <NbCore.hpp>

#include <random>
#include <cmath>
#include <limits>

#include <Vector.hpp>


#include "Math/Vector2.hpp"
#include "Math/Vector3.hpp"

#include "IRenderAPI.hpp"
#include "Manager/ResourceManager.hpp"

//
#include "OpenGL/Placeholder.hpp"
//
namespace nb::Renderer
{

    struct SSAOConfig
    {
        float bias = 0.025;
        float radius = 0.5;
    };

    class SSAO
    {
    public:
        static constexpr uint32 NOISE_DIMENSION = 4;
        static constexpr uint32 NOISE_SAMPLES   = NOISE_DIMENSION * NOISE_DIMENSION;
        static constexpr float  LERP_MIN        = 0.1f;
        static constexpr float  LERP_MAX        = 1.0f;
        static constexpr float  RADIUS_MIN      = -1.0f;
        static constexpr float  RADIUS_MAX      = 1.0f;

        explicit SSAO(
            IRenderAPI* renderApi,
            uint32      width,
            uint32      height,
            uint8       kernelSizeValue = 64
        ) noexcept
            : api(renderApi)
            , kernelSize(kernelSizeValue)
        {

            if (!api)
            {
                nb::Error::ErrorManager::instance().report(
                    nb::Error::Type::FATAL, "Render API is null in SSAO constructor"
                );
                return;
            }

            generateKernel();
            generateNoiseTexture();
            initFramebuffers(width, height);

            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::INFO, "SSAO module initialized")
                .with("kernelSize", static_cast<uint32>(kernelSize));
        }

        ~SSAO() = default;

        void resize(
            uint32 width,
            uint32 height
        ) noexcept
        {
            if (width == 0 || height == 0)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::WARNING, "SSAO resize with zero dimensions")
                    .with("w", width)
                    .with("h", height);
                return;
            }
            initFramebuffers(width, height);
        }

        NB_NODISCARD uint64 process(
            ResMan::ResourceManager* res,
            uint64                   gPositionHandle,
            uint64                   gNormalHandle,
            Camera*                  cam
        ) noexcept
        {
            if (!res || !cam)
            {
                return 0;
            }

            ssaoFbo->bind();
            auto ssaoShader = res->getResource<Shader>("ssao_main.shader");
            if (ssaoShader)
            {
                ssaoShader->use();
                ssaoShader->setUniformUint64("gPosition", gPositionHandle);
                ssaoShader->setUniformUint64("gNormal", gNormalHandle);
                ssaoShader->setUniformUint64("texNoise", noiseTexture);

                ssaoShader->setUniformFloat("radius", config.radius);
                ssaoShader->setUniformFloat("bias", config.bias);

                ssaoShader->setUniformMat4("projection", cam->getProjection());
                ssaoShader->setUniformVec3Array("samples", kernel.data(), kernelSize);
                ssaoShader->setUniformVec2("noiseScale", noiseScale);

                drawFullscreenTriangle();
            }
            ssaoFbo->unBind();

            blurFbo->bind();
            auto blurShader = res->getResource<Shader>("ssao_blur.shader");
            if (blurShader)
            {
                blurShader->use();
                blurShader->setUniformUint64("ssaoInput", ssaoFbo->getTextureHandle());
                drawFullscreenTriangle();
            }
            blurFbo->unBind();

            return blurFbo->getTextureHandle();
        }

        const SSAOConfig& getConfig() const noexcept
        {
            return config;
        }

        SSAOConfig& getConfig() noexcept
        {
            return config;
        }

    private:
        void generateKernel() noexcept
        {
            kernel.clear();

            std::random_device                    rd;
            std::mt19937                          gen(rd());
            std::uniform_real_distribution<float> disRadius(RADIUS_MIN, RADIUS_MAX);
            std::uniform_real_distribution<float> disZ(0.0f, 1.0f);

            for (uint32 i = 0; i < kernelSize; ++i)
            {
                nb::Math::Vector3<float> sample(disRadius(gen), disRadius(gen), disZ(gen));

                sample.normalize();

                float scale = static_cast<float>(i) / static_cast<float>(kernelSize);
                scale       = LERP_MIN + (scale * scale) * (LERP_MAX - LERP_MIN);

                kernel.pushBack(sample * scale);
            }
        }

        void generateNoiseTexture() noexcept
        {
            nbstl::Vector<nb::Math::Vector3<float>> noiseData;

            std::random_device                    rd;
            std::mt19937                          gen(rd());
            std::uniform_real_distribution<float> dis(RADIUS_MIN, RADIUS_MAX);

            for (uint32 i = 0; i < NOISE_SAMPLES; ++i)
            {
                noiseData.pushBack({dis(gen), dis(gen), 0.0f});
            }

            // В идеале создание текстуры должно идти через IRenderAPI
            // noiseTexture = api->createTexture(...);
            noiseTexture = OpenGl::createPlaceholderForNoise();

            if (noiseTexture == 0)
            {
                nb::Error::ErrorManager::instance().report(
                    nb::Error::Type::FATAL, "Failed to create SSAO noise texture"
                );
            }
        }

        void initFramebuffers(
            uint32 w,
            uint32 h
        ) noexcept
        {
            noiseScale = {
                static_cast<float>(w) / static_cast<float>(NOISE_DIMENSION),
                static_cast<float>(h) / static_cast<float>(NOISE_DIMENSION)
            };

            ssaoFbo = api->createFrameBuffer(w, h);
            if (ssaoFbo)
            {
                ssaoFbo->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR);
                ssaoFbo->finalize();
                ssaoFbo->setDrawBuffers(1);
            }

            blurFbo = api->createFrameBuffer(w, h);
            if (blurFbo)
            {
                blurFbo->addTextureAttachment(IFrameBuffer::TextureAttachment::COLOR);
                blurFbo->finalize();
                blurFbo->setDrawBuffers(1);
            }

            if (!ssaoFbo || !blurFbo)
            {
                nb::Error::ErrorManager::instance()
                    .report(nb::Error::Type::FATAL, "Failed to initialize SSAO framebuffers")
                    .with("width", w)
                    .with("height", h);
            }
        }

        void drawFullscreenTriangle() const noexcept
        {
            static uint32 emptyVao = 0;
            if (emptyVao == 0)
            {
                glCreateVertexArrays(1, &emptyVao);
            }

            glBindVertexArray(emptyVao);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }

        

    private:
        uint8       kernelSize = 0;

        nbstl::Vector<nb::Math::Vector3<float>> kernel;
        uint64                                  noiseTexture = 0;

        Ref<IFrameBuffer> ssaoFbo = nullptr;
        Ref<IFrameBuffer> blurFbo = nullptr;

        nb::Math::Vector2<float> noiseScale{0.0f, 0.0f};
        IRenderAPI*              api = nullptr;

        SSAOConfig config = {};

    };
} 

#endif 