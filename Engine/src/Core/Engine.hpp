#ifndef SRC_CORE_ENGINE_HPP
#define SRC_CORE_ENGINE_HPP

#ifdef _WIN32
#   ifdef ENGINE_EXPORTS
#       define ENGINE_API __declspec(dllexport)
#   else
#       define ENGINE_API __declspec(dllimport)
#   endif
#else
#   define ENGINE_API
#endif



#include "../Subsystems.hpp"

#include "ShaderSystem.hpp"

#include "../Input/Input.hpp"
#include "../Input/Keyboard.hpp"
#include "../Input/Mouse.hpp"

#include "../Renderer/Camera.hpp"

#include <memory>
#include <mutex>
#include <queue>
#include <future>

namespace nb
{
    namespace Core
    {
        class Engine
        {
        public:

            enum class Mode : uint8_t
            {
                EDITOR,
                GAME
            };

            Engine(const HWND& windowHwnd);
            ~Engine() = default;

            void bufferizeInput(const MSG& msg) const noexcept;
            void processInput() const noexcept;
            void setHandleInput(bool var) { handleInput = var; }

            bool run(Input::MouseDelta mouseDelta, Input::MouseButtons buttons);
            void handleGameMode(nb::Math::Vector3<float> &camDir, float deltaTime) noexcept;
            void handleEditorMode() noexcept;


            void rayPick(const uint32_t x, const uint32_t y) const noexcept;
            
            Mode getMode() const noexcept;

            inline static const HWND& getLinkedHwnd() noexcept { return hwnd; }
            inline Math::Vector3<float> getCameraPos() const noexcept { return renderer->getCamera()->getPosition(); }
            inline Math::Vector3<float> getCameraDirection() const noexcept { return renderer->getCamera()->getDirection(); }
            inline std::shared_ptr<Renderer::SceneGraph> getScene() const noexcept { return renderer->getScene(); }
            inline Ref<nb::Renderer::Renderer> getRenderer() noexcept { return renderer; }

			template<typename F>
			void invokeAsync(F&& func)
			{
				std::lock_guard lk(mtx);
				queue.emplace(std::forward<F>(func));
			}

			template<typename F>
			auto invokeAsyncWithResult(F&& func) -> std::future<decltype(func(*this))>
			{
				using R = decltype(func(*this));
				auto promise = std::make_shared<std::promise<R>>();
				auto fut = promise->get_future();

				invokeAsync([f = std::forward<F>(func), promise](Engine& e) mutable
				{
					if constexpr (std::is_void_v<R>)
					{
						f(e);
						promise->set_value();
					}
					else
					{
						promise->set_value(f(e));
					}
				});

				return fut;
			}

			// 
			template<typename F>
			void invokeSync(F&& func)
			{
				invokeAsyncWithResult([func = std::forward<F>(func)](Engine& e)
				{
					func(e);
				}).get(); 
			}

			template<typename F>
			auto invokeSyncWithResult(F&& func) -> decltype(func(*this))
			{
				return invokeAsyncWithResult(std::forward<F>(func)).get();
			}

			void processCommands()
			{
				MessageQueue local;
				{
					std::lock_guard lk(mtx);
					std::swap(local, queue);
				}
				while (!local.empty())
				{
					local.front()(*this);
					local.pop();
				}
			}

            ShaderSystem& getShaderSystem() noexcept;

        private:
			using MessageQueue = std::queue<std::function<void(Engine&)>>;

            ShaderSystem					shaderSystem;

            Mode							mode            = Mode::EDITOR;

            std::unique_ptr<Subsystems>		subSystems      = std::make_unique<Subsystems>();
            Ref<nb::Renderer::Renderer>		renderer        = nullptr;
            Ref<nb::Input::Keyboard>		keyboard        = nullptr;
            Ref<nb::Input::Mouse>			mouse           = nullptr;
            bool							isRunning       = true;
            bool							handleInput     = true;
            
            nb::Renderer::Camera			cam;
            // temp
            Ref<nb::Input::Input>			input           = nullptr;

            Input::MouseDelta				mouseDelta      = {};
            Input::MouseButtons				buttons         = {};

            inline static HWND				hwnd            = nullptr;

			std::mutex						mtx;
			MessageQueue					queue;

        };
    };
};

#endif