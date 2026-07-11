// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "SceneWindow.hpp"
#include <Common/StringUtils.hpp>

#include <Error/ErrorManager.hpp>
#include <LayoutBuilder.hpp>

#include <Widgets/Button.hpp>
#include <Widgets/SpinBox.hpp>

#include "SceneController.hpp"

namespace sdk
{
    SceneWindow::SceneWindow(WindowInterface::IWindow* parent) noexcept
        : parentWindow(parent)
        , sceneController(nullptr)
    {
    }

    void SceneWindow::initialize() noexcept
    {
        if (!parentWindow)
        {
            nb::Error::ErrorManager::instance().report(
                nb::Error::Type::FATAL, "Parent window is null during SceneWindow initialization"
            );
            return;
        }

        sceneTabWindow = std::make_shared<Win32Window::ChildWindow>(parentWindow, true);
        sceneTabWindow->setTitle(
            Utils::toWstring(Localization::Translation::fromKey("Ui.Editor.SceneTab.Title"))
        );

        sceneToolbar = std::make_shared<Win32Window::ChildWindow>(sceneTabWindow.get());
        sceneToolbar->setTitle(
            Utils::toWstring(Localization::Translation::fromKey("Ui.Editor.SceneToolbar.Title"))
        );

        sceneViewportWindow =
            std::make_shared<Win32Window::ChildWindow>(sceneTabWindow.get(), true);
        sceneViewportWindow->setTitle(
            Utils::toWstring(Localization::Translation::fromKey("Ui.Editor.Scene.Title"))
        );

        auto popup = std::make_unique<nbui::PopupMenu>();
        popup->addItem(
            Utils::toWstring(Localization::Translation::fromKey("Ui.Scene.RandomizeAngle")),
            nbui::IconType::None,
            [this]()-> void
            {
                if(sceneController && sceneController->getActiveNode().isValid())
                {
                    // TODO: replace if more generators used
                    static std::mt19937 gen([]() {
                        std::random_device rd;
                        return rd();
                    }());

                    std::uniform_real_distribution<float> dist(0.0f, nb::Math::Constants::PI * 2.0f); 

                    float rx = dist(gen);
                    float ry = dist(gen);
                    float rz = dist(gen);

                    auto node = sceneController->getActiveNode();
                    auto& transform = node.getComponent<TransformComponent>();
                    
                    transform.rotation = nb::Math::Quaternion<float>::eulerToQuaternionXYZ(rx, ry, rz);         
                    transform.dirty = true;
                }
            }
        );

        sceneToolbar->setContextMenu(std::move(popup));

        nb::Error::ErrorManager::instance().report(
            nb::Error::Type::INFO, "SceneWindow components created successfully"
        );
    }   

    void SceneWindow::handleResize(const NbRect<int>& rect) noexcept
    {
        if (!sceneTabWindow || !sceneToolbar || !sceneViewportWindow)
        {
            return;
        }

        const NbPoint<int>& scenePos  = sceneTabWindow->getPosition();
        const NbSize<int>&  sceneSize = sceneTabWindow->getSize();

        sceneToolbar->setPosition({scenePos.x, scenePos.y});
        sceneToolbar->setSize({sceneSize.width, UIConstants::TOOLBAR_HEIGHT});

        sceneViewportWindow->setPosition({scenePos.x, scenePos.y + UIConstants::TOOLBAR_HEIGHT});
        sceneViewportWindow->setSize(
            {sceneSize.width, sceneSize.height - UIConstants::TOOLBAR_HEIGHT}
        );

        HWND viewportHandle = sceneViewportWindow->getHandle().as<HWND>();
        if (viewportHandle)
        {
            LONG_PTR viewportStyle = GetWindowLongPtr(viewportHandle, GWL_STYLE);
            SetWindowLongPtr(
                viewportHandle, GWL_STYLE, viewportStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
            );
        }
    }

    void SceneWindow::attachController(std::shared_ptr<SceneController> controller) noexcept
    {
        sceneController = controller;
        sceneToolbar->getLayoutRoot()->addChild(buildToolbar().build());
    }

    nbui::LayoutBuilder SceneWindow::buildToolbar() noexcept
{
    using namespace nbui;
    
    return LayoutBuilder::hBox()
        .relativeWidth(1.0f)
        .relativeHeight(1.0f)
        .background(NbColor{24, 24, 24}) 
        .padding(Padding<int>{4, 10, 4, 10})
        .spacing(4)
        
        .buttonGroupOnlyOne()
            .relativeHeight(1.0f)
            .autoWidth()
            .child(
                LayoutBuilder::widget(new Widgets::Button())
                    .text(L"Lit")
                    .absoluteWidth(36)
                    .relativeHeight(1.0f)
                    .background({31, 56, 92}, LayoutBuilder::StateStyle::ACTIVE)
                    .onEvent(&Widgets::Button::onReleasedSignal, [this]() {
                        this->sceneController->setViewMode(ViewMode::Lit);
                    }) 
            )
            .child(
                LayoutBuilder::widget(new Widgets::Button())
                    .text(L"Wireframe")
                    .absoluteWidth(85)
                    .relativeHeight(1.0f)
                    .onEvent(&Widgets::Button::onReleasedSignal, [this]() {
                        this->sceneController->setViewMode(ViewMode::Wireframe);
                    })
            )
            .checkedGroupIndex(true, 0) 
        .endGroup()

        .child(LayoutBuilder::spacer())

        .child(
            LayoutBuilder::widget(new Widgets::CheckBox())
                .text(L"⌗") 
                .absoluteWidth(45)
                .relativeHeight(1.0f)
                .onEvent(&Widgets::CheckBox::onCheckStateChanged, [this](bool checked) {
                    sdk::GridSnapping settings = this->sceneController->getSnapper().getGridSnapping();
                    settings.translationEnabled = checked;
                    this->sceneController->getSnapper().setGridSnapping(settings);
                })
        )
        .child(
            LayoutBuilder::hBox()
                .absoluteWidth(60)
                .relativeHeight(1.0f)
                .background({38, 38, 38})
                .border(1, Border::Style::SOLID, {55, 55, 55})
                .child(
                    LayoutBuilder::floatSlider()
                        .relativeWidth(1.0f)  
                        .relativeHeight(1.0f) 
                        .color({210, 210, 210})
                        .textAlignment({.textAlignment = TextAlignment::CENTER})
                        .checked(this->sceneController->getSnapper().getGridSnapping().translationEnabled)
                        .apply<Widgets::Slider<float>>([this](Widgets::Slider<float>* c)
                        {
                            c->setRange(0.0f, 100.0f, 0.1f);
                            c->setValue(0.0f);
                            c->bind(
                                [this]() -> float {
                                    sdk::GridSnapping settings = this->sceneController->getSnapper().getGridSnapping();
                                    return settings.translationFactor;
                                },
                                [this](float value) -> void {
                                    sdk::GridSnapping settings = this->sceneController->getSnapper().getGridSnapping();
                                    settings.translationFactor = value;
                                    this->sceneController->getSnapper().setGridSnapping(settings);
                                }
                            );
                        })
                )
        )
        .child(
            LayoutBuilder::label(L"м")
                .color({130, 130, 130})
                .absoluteWidth(15)
                .relativeHeight(1.0f)
                .textAlignment({.textAlignment = TextAlignment::CENTER})
        )

        .child(
            LayoutBuilder::label(L"|")
                .color({50, 50, 50})
                .absoluteWidth(15)
                .relativeHeight(1.0f)
                .textAlignment({.textAlignment = TextAlignment::CENTER})
        )

        .child(
            LayoutBuilder::widget(new Widgets::CheckBox())
                .text(L"↻") 
                .absoluteWidth(45)
                .relativeHeight(1.0f)
                .checked(this->sceneController->getSnapper().getGridSnapping().rotationEnabled)
                .onEvent(&Widgets::CheckBox::onCheckStateChanged, [this](bool checked) {
                    sdk::GridSnapping settings = this->sceneController->getSnapper().getGridSnapping();
                    settings.rotationEnabled = checked;
                    this->sceneController->getSnapper().setGridSnapping(settings);
                })
        )
        .child(
            LayoutBuilder::hBox()
                .absoluteWidth(60)
                .relativeHeight(1.0f)
                .background({38, 38, 38})
                .border(1, Border::Style::SOLID, {55, 55, 55})
                .child(
                    LayoutBuilder::floatSlider()
                        .relativeWidth(1.0f)  
                        .relativeHeight(1.0f) 
                        .color({210, 210, 210})
                        .textAlignment({.textAlignment = TextAlignment::CENTER})
                        .apply<Widgets::Slider<float>>([this](Widgets::Slider<float>* c)
                        {
                            c->setRange(0.0f, 100.0f, 0.1f);
                            c->setValue(0.0f);
                            c->bind(
                                [this]() -> float {
                                    sdk::GridSnapping settings = this->sceneController->getSnapper().getGridSnapping();
                                    return settings.rotationFactor;
                                },
                                [this](float value) -> void {
                                    sdk::GridSnapping settings = this->sceneController->getSnapper().getGridSnapping();
                                    settings.rotationFactor = value;
                                    this->sceneController->getSnapper().setGridSnapping(settings);
                                }
                            );
                        })
                )
        )
        .child(
            LayoutBuilder::label(L"°")
                .color({130, 130, 130})
                .absoluteWidth(15)
                .relativeHeight(1.0f)
                .textAlignment({.textAlignment = TextAlignment::CENTER})
        )

        .child(
            LayoutBuilder::label(L"|")
                .color({50, 50, 50})
                .absoluteWidth(15)
                .relativeHeight(1.0f)
                .textAlignment({.textAlignment = TextAlignment::CENTER})
        )

        .child(
            LayoutBuilder::widget(new Widgets::CheckBox())
                .text(L"⤢") 
                .absoluteWidth(45)
                .relativeHeight(1.0f)
                .background({33, 33, 33})
                .checked(this->sceneController->getSnapper().getGridSnapping().scaleEnabled)
                .onEvent(&Widgets::CheckBox::onCheckStateChanged, [this](bool checked) {
                    sdk::GridSnapping settings = this->sceneController->getSnapper().getGridSnapping();
                    settings.scaleEnabled = checked;
                    this->sceneController->getSnapper().setGridSnapping(settings);
                })
        )
        .child(
            LayoutBuilder::hBox()
                .absoluteWidth(60)
                .relativeHeight(1.0f)
                .background({38, 38, 38})
                .border(1, Border::Style::SOLID, {55, 55, 55})
                .child(
                    LayoutBuilder::floatSlider()
                        .relativeWidth(1.0f)  
                        .relativeHeight(1.0f) 
                        .color({210, 210, 210})
                        .textAlignment({.textAlignment = TextAlignment::CENTER})
                        .apply<Widgets::Slider<float>>([this](Widgets::Slider<float>* c)
                        {
                            c->setRange(0.0f, 100.0f, 0.1f);
                            c->setValue(0.0f);
                            c->bind(
                                [this]() -> float {
                                    sdk::GridSnapping settings = this->sceneController->getSnapper().getGridSnapping();
                                    return settings.scaleFactor;
                                },
                                [this](float value) -> void {
                                    sdk::GridSnapping settings = this->sceneController->getSnapper().getGridSnapping();
                                    settings.scaleFactor = value;
                                    this->sceneController->getSnapper().setGridSnapping(settings);
                                }
                            );
                        })
                )
        );
}
} // namespace sdk