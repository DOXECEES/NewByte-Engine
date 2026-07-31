#ifndef SRC_RENDERER_POSTPROCESS_HPP
#define SRC_RENDERER_POSTPROCESS_HPP

#include <NbCore.hpp>

#include "Renderer/IRenderAPI.hpp"
#include "Renderer/IUniformBuffer.hpp"

#include <NonOwningPtr.hpp>

#include <bitset>
#include <array>
#include <memory>

namespace nb::Renderer
{
    enum class PostProcessEffect : size_t
    {
        Fxaa = 0,
        Dof = 1,
        Count
    };

    struct PostProcessEffectConfig
    {
        virtual ~PostProcessEffectConfig() = default;
        virtual PostProcessEffect getPostEffectType() const noexcept = 0; 
    };

    struct FxaaGpuData 
    {
        int reserved;
    };

    struct Fxaa : public PostProcessEffectConfig
    {
        static constexpr PostProcessEffect EffectType = PostProcessEffect::Fxaa;
        using EffectGpuData = FxaaGpuData;
 
        PostProcessEffect getPostEffectType() const noexcept override { return EffectType; }
    };

    class PostProcess
    {
    public:

        template <typename T>
        using UboPtr = std::unique_ptr<IUniformBuffer<T>>;

        using UboBasePtr = std::unique_ptr<IUniformBufferBase>;
        
        constexpr static size_t MAX_COUNT_OF_POST_PROCESS_EFFECTS = static_cast<size_t>(PostProcessEffect::Count);
    
        NB_NON_COPYMOVABLE(PostProcess);
        
        PostProcess(const nbstl::NonOwningPtr<IRenderAPI> renderApi) noexcept;
        ~PostProcess() = default;

        void addEffect(PostProcessEffect effect);
        void removeEffect(PostProcessEffect effect);
        bool isEffectActive(PostProcessEffect effect) const;
        
        template<typename T>
        IUniformBuffer<typename T::EffectGpuData>* getUniformBuffer(PostProcessEffect effect) const
        {
            const size_t index = static_cast<size_t>(effect); 
            if(index >= MAX_COUNT_OF_POST_PROCESS_EFFECTS || !isEffectActive(effect))
            {
                return nullptr;    
            }

            using TargetUboType = IUniformBuffer<typename T::EffectGpuData>;

            #ifdef NB_DEBUG 
                auto* casted = dynamic_cast<TargetUboType*>(uniformBuffers[index].get());
                assert(casted != nullptr && "Incorrect type casting!");
                return casted;
            #else
                return static_cast<TargetUboType*>(uniformBuffers[index].get());
            #endif
        }


        template<typename T>
        void addEffect()
        {
            size_t index = static_cast<size_t>(T::EffectType);
            if(!config[index])
            {
                config[index] = std::make_unique<T>();
            }

            if(!uniformBuffers[index])
            {
                uniformBuffers[index] = api->createUniformBuffer<typename T::EffectGpuData>();
            }

            addEffect(T::EffectType);
        }

        template<typename T>
        void removeEffect()
        {
            removeEffect(T::EffectType);
        }

        template<typename T>
        bool isEffectActive() const
        {
            return isEffectActive(T::EffectType);
        }

        template<typename T>
        IUniformBuffer<typename T::EffectGpuData>* getUniformBuffer() const
        {
            return getUniformBuffer<T>(T::EffectType);
        }


        template<typename T>
        T* getEffectConfig()
        {
            return getEffectConfigInternal<T>(T::EffectType);
        }

        template<typename T>
        const T* getEffectConfig() const
        {
            return getEffectConfigInternal<T>(T::EffectType);
        }

    private:
        template<typename T>
        T* getEffectConfigInternal(PostProcessEffect effect)
        {
            size_t index = static_cast<size_t>(effect);
            if (index >= MAX_COUNT_OF_POST_PROCESS_EFFECTS || !config[index])
            {
                return nullptr;
            }

            #ifdef NB_DEBUG
                auto* casted = dynamic_cast<T*>(config[index].get());
                assert(casted != nullptr && "Incorrect type casting!");
                return casted;
            #else
                return static_cast<T*>(config[index].get());
            #endif
        }

        template<typename T>
        const T* getEffectConfigInternal(PostProcessEffect effect) const
        {
            size_t index = static_cast<size_t>(effect);
            if (index >= MAX_COUNT_OF_POST_PROCESS_EFFECTS || !config[index])
            {
                return nullptr;
            }

            #ifdef NB_DEBUG
                auto* casted = dynamic_cast<const T*>(config[index].get());
                assert(casted != nullptr && "Incorrect type casting!");
                return casted;
            #else
                return static_cast<const T*>(config[index].get());
            #endif
        }


    private:
        std::bitset<MAX_COUNT_OF_POST_PROCESS_EFFECTS> activeEffects;
        std::array<std::unique_ptr<PostProcessEffectConfig>, MAX_COUNT_OF_POST_PROCESS_EFFECTS> config;
        std::array<UboBasePtr, MAX_COUNT_OF_POST_PROCESS_EFFECTS> uniformBuffers; 

        nbstl::NonOwningPtr<IRenderAPI> api;

    };
};


#endif