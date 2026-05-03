#ifndef SRC_RENDERER_GBUFFER_HPP
#define SRC_RENDERER_GBUFFER_HPP

#include "IRenderAPI.hpp"
#include "IFrameBuffer.hpp"

#include <NonOwningPtr.hpp>

#include <memory>

namespace nb::Renderer
{
	class GBuffer
	{
    public:
            GBuffer(
                nbstl::NonOwningPtr<IRenderAPI> api,
                uint32                          width,
                uint32                          height
            ) noexcept;
	
			const Ref<IFrameBuffer>& getFramebuffer() const noexcept;

	private:

		Ref<IFrameBuffer> framebuffer;
	};
};

#endif