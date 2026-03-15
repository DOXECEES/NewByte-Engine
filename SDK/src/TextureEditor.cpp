#include "TextureEditor.hpp"

#include <Widgets/Button.hpp>
#include <Widgets/Label.hpp>
#include <Widgets/Slider.hpp>
#include <Widgets/ComboBox.hpp>


#include <memory>



TextureEditor::TextureEditor(
    WindowInterface::IWindow* parent,
    nb::Core::Engine* engine,
    nbstl::NonOwningPtr<nb::Resource::TextureAsset> texture
) 
    : engine(engine) 
{
    textureEditorWindow =
        std::make_shared<Win32Window::ModalWindow>(NbSize<int>{1100, 700}, parent);
    textureEditorWindow->setTitle(L"Content Browser");

    previewWindow = std::make_shared<Win32Window::ChildWindow>(textureEditorWindow.get(), true);
    previewWindow->setTitle(L"Preview Texture");

    subscribe(*textureEditorWindow, &Win32Window::ModalWindow::onSizeChanged, [this](const NbSize<int>& size) {
            previewWindow->setSize({static_cast<int>(size.width * 0.7f), size.height - 32});
    });

    sharedContext = engine->getRenderer()->createSharedContextForWindow(previewWindow->getHandle().as<HWND>());
    previewWindow->setRenderable(false);

    inspectorWindow = std::make_shared<Win32Window::ChildWindow>(textureEditorWindow.get());
    inspectorWindow->setTitle(L"Inspector");

    subscribe(*textureEditorWindow, &Win32Window::ModalWindow::onSizeChanged, [this](const NbSize<int>& size) {
        this->handleResize(size);
    });

    handleResize(textureEditorWindow->getClientSize());

    textureEditorWindow->getLayoutRoot()->addChild(std::move(buildEditorUI()));
    inspectorWindow->getLayoutRoot()->addChild(std::move(buildInspectorUI()));

    settings.source = texture->getInternalTexture().get();
}

void TextureEditor::handleResize(const NbSize<int>& size)
{
    const int previewWidth = static_cast<int>(size.width * Metrics::previewRatio);
    const int contentHeight = size.height - Metrics::toolbarHeight;
    const int topOffset = Metrics::titleBarHeight + Metrics::toolbarHeight;

    const NbRect<int> clientArea = textureEditorWindow->getClientRect();

    previewWindow->setSize({previewWidth, contentHeight});
    previewWindow->setPosition({Metrics::padding, topOffset});

    inspectorWindow->setSize({size.width - previewWidth + Metrics::padding, contentHeight});
    inspectorWindow->setPosition({previewWidth, topOffset});
}

std::unique_ptr<NNsLayout::LayoutNode> TextureEditor::buildEditorUI()
{
    using namespace nbui;
    auto rootUI =
        LayoutBuilder::vBox()
            .style(
                [](NNsLayout::LayoutStyle& s)
                {
                    s.width = 1.0f;
                    s.height = 1.0f;
                    s.widthSizeType = NNsLayout::SizeType::RELATIVE;
                    s.heightSizeType = NNsLayout::SizeType::RELATIVE;
                    s.color = NbColor{30, 30, 30};
                }
            )
            .child(
                LayoutBuilder::toolbar()
                    .buttonGroupOnlyOne()
                              .relativeHeight(1.0f)
                              .absoluteWidth(120)
                              .child(
                                  LayoutBuilder::widget(new Widgets::Button())
                                      .text(L"2D")
                                      .absoluteWidth(50)
                                      .relativeHeight(1.0f)
                              )
                              .child(
                                  LayoutBuilder::widget(new Widgets::Button())
                                      .text(L"3D")
                                      .absoluteWidth(50)
                                      .relativeHeight(1.0f)
                              )
                              
                            .endGroup()

                        .buttonGroupMultiple()
                              .relativeHeight(1.0f)
                              .relativeWidth(0.5f)
                              .child(
                                  LayoutBuilder::widget(new Widgets::Button())
                                      .text(L"R")
                                      .absoluteWidth(50)
                                      .relativeHeight(1.0f)
                                      .background({180, 60, 60}, LayoutBuilder::StateStyle::ACTIVE)
                                      .onEvent(&Widgets::Button::onCheckedChangedSignal, [this](bool flag) {
                                         settings.channelMask.x = flag ? 1.0f : 0.0f;
                                        })
                              )
                              .child(
                                  LayoutBuilder::widget(new Widgets::Button())
                                      .text(L"G")
                                      .absoluteWidth(50)
                                      .relativeHeight(1.0f)
                                      .background({60, 150, 80}, LayoutBuilder::StateStyle::ACTIVE)
                                        .onEvent(&Widgets::Button::onCheckedChangedSignal, [this](bool flag) {
                                            settings.channelMask.y = flag ? 1.0f : 0.0f;
                                        })
                              )
                              .child(
                                  LayoutBuilder::widget(new Widgets::Button())
                                      .text(L"B")
                                      .absoluteWidth(50)
                                      .relativeHeight(1.0f)     
                                      .background({60, 100, 180}, LayoutBuilder::StateStyle::ACTIVE)
                                        .onEvent(&Widgets::Button::onCheckedChangedSignal, [this](bool flag) {
                                            settings.channelMask.z = flag ? 1.0f : 0.0f;
                                        })
                              )
                              .child(
                                  LayoutBuilder::widget(new Widgets::Button())
                                      .text(L"A")
                                      .absoluteWidth(50)
                                      .relativeHeight(1.0f)
                                    .background({150, 150, 150}, LayoutBuilder::StateStyle::ACTIVE)

                              )
                            .checkedGroupIndex(true, 0)
                            .checkedGroupIndex(true, 1)
                            .checkedGroupIndex(true, 2)
                            .checkedGroupIndex(true, 3)

                            .endGroup()

                      )

                    
            
                

            //// Верхний Тулбар
            //.child(
            //    LayoutBuilder::hBox()
            //        .absoluteHeight(50)
            //        .background(NbColor{45, 45, 45})
                    
                    
            
            // Область со скроллом (наш FlowLayout)
            // .child(
            //     LayoutBuilder::vBox()
            //         .style(
            //             [](NNsLayout::LayoutStyle& s)
            //             {
            //                 s.width = 1.0f;
            //                 s.height = 1.0f;
            //                 s.widthSizeType = NNsLayout::SizeType::RELATIVE;
            //                 s.heightSizeType = NNsLayout::SizeType::RELATIVE;
            //             }
            //         )
            //         .child(std::move(assetFlow))       // Добавляем готовую сетку ассетов
            //         .child(LayoutBuilder::spacer()) // Прижимает сетку к верху
            // )
            // Статус-бар
            // .child(
            //     LayoutBuilder::hBox()
            //         .absoluteHeight(30)
            //         .relativeWidth(1.0f)
            //         .background(NbColor{20, 20, 20})
            //         .child(
            //             LayoutBuilder::label(L"  Total Resources Loaded: ...")
            //                 .fontSize(11)
            //                 .absoluteHeight(30)
            //                 .relativeWidth(1.0f)
            //         )
            // )
            .build();

    // 6. Устанавливаем UI в окно и показываем
    
    return rootUI;
}

std::unique_ptr<NNsLayout::LayoutNode> TextureEditor::buildInspectorUI()
{
    using namespace nbui;
       auto inspectorUi =
        LayoutBuilder::vBox()
            .padding({10, 10, 0, 10})
            .relativeWidth(1.0f)  // Растягиваем на всю ширину окна инспектора
            .relativeHeight(1.0f) // Растягиваем на всю высоту

            // --- СЕКЦИЯ: ИНФОРМАЦИЯ ---
            .child(
                LayoutBuilder::vBox()
                    .relativeWidth(1.0f)
                    .autoHeight() // Высота всей секции метаданных
                    .child(
                        LayoutBuilder::label(L"METADATA")
                            .fontSize(12)
                            .color({150, 150, 150})
                            .absoluteHeight(20)
                            .relativeWidth(1.0f)
                    )
                    .child(
                        LayoutBuilder::label(L"Format: RGBA8_UNORM")
                            .fontSize(14)
                            .absoluteHeight(20)
                            .relativeWidth(1.0f)
                    )
                    .child(
                        LayoutBuilder::label(L"Size: 2048 x 2048")
                            .fontSize(14)
                            .absoluteHeight(20)
                            .relativeWidth(1.0f)
                    )
                    .child(
                        LayoutBuilder::label(L"Mips: 11")
                            .fontSize(14)
                            .absoluteHeight(20)
                            .relativeWidth(1.0f)
                    )
            )

            // Разделитель (фиксированная высота 2 пикселя)
            .child(
                LayoutBuilder::hBox()
                    .relativeWidth(1.0f)
                    .absoluteHeight(2)
                    .background({60, 60, 60})
                    .margin({10, 0, 10, 0}) // Отступы сверху и снизу
            )

            // --- СЕКЦИЯ: КАНАЛЫ (RGBA) ---
            .child(
                LayoutBuilder::vBox()
                    .relativeWidth(1.0f)
                    .absoluteHeight(60)
                    .child(
                        LayoutBuilder::label(L"CHANNELS")
                            .fontSize(12)
                            .color({150, 150, 150})
                            .absoluteHeight(20)
                            .relativeWidth(1.0f)
                    )
                    .child(
                        LayoutBuilder::hBox()
                            .relativeWidth(1.0f)
                            .absoluteHeight(35)
                            .buttonGroupMultiple()
                            .relativeWidth(1.0f)
                            .absoluteHeight(35)
                            .child(
                                LayoutBuilder::widget(new Widgets::Button())
                                    .text(L"R")
                                    .relativeWidth(0.25f)
                                    .relativeHeight(1.0f)
                                    .background({220, 80, 80}, LayoutBuilder::StateStyle::ACTIVE)
                            )
                            .child(
                                LayoutBuilder::widget(new Widgets::Button())
                                    .text(L"G")
                                    .relativeWidth(0.25f)
                                    .relativeHeight(1.0f)
                                    .background({110, 180, 120}, LayoutBuilder::StateStyle::ACTIVE)
                            )
                            .child(
                                LayoutBuilder::widget(new Widgets::Button())
                                    .text(L"B")
                                    .relativeWidth(0.25f)
                                    .relativeHeight(1.0f)
                                    .background({80, 140, 220}, LayoutBuilder::StateStyle::ACTIVE)
                            )
                            .child(
                                LayoutBuilder::widget(new Widgets::Button())
                                    .text(L"A")
                                    .relativeWidth(0.25f)
                                    .relativeHeight(1.0f)
                                    .background({210, 210, 210}, LayoutBuilder::StateStyle::ACTIVE)
                            )
                            .endGroup()
                    )
            )

            // Еще один разделитель
            .child(
                LayoutBuilder::hBox()
                    .relativeWidth(1.0f)
                    .absoluteHeight(2)
                    .background({60, 60, 60})
                    .margin({10, 0, 10, 0}) // Отступы сверху и снизу

            )

            // --- СЕКЦИЯ: ПАРАМЕТРЫ (Exposure, Gamma, Filtering) ---
            .child(
                LayoutBuilder::vBox()
                    .relativeWidth(1.0f)
                    .absoluteHeight(110) // Достаточно места для заголовка и 3 строк
                    .child(
                        LayoutBuilder::label(L"VISUALS")
                            .fontSize(12)
                            .color({150, 150, 150})
                            .absoluteHeight(20)
                            .relativeWidth(1.0f)
                    )
                    // Exposure
                    .child(
                        LayoutBuilder::hBox()
                            .relativeWidth(1.0f)
                            .absoluteHeight(30)
                            .child(
                                LayoutBuilder::label(L"Exposure")
                                    .relativeWidth(0.4f)
                                    .relativeHeight(1.0f)
                            )
                            .child(
                                LayoutBuilder::widget(new Widgets::Slider<float>())
                                     .apply<Widgets::Slider<float>>([this](auto* s) {
                                        s->setRange(0.0f, 10.0f, 0.1f);
                                        s->bind(
                                            [this]() { return settings.exposure; },
                                            [this](float v) { settings.exposure = v; }
                                        );
                                    })
                                    .relativeWidth(0.6f)
                                    .relativeHeight(1.0f)
                            )
                    )
                    // Gamma
                    .child(
                        LayoutBuilder::hBox()
                            .relativeWidth(1.0f)
                            .absoluteHeight(30)
                            .child(
                                LayoutBuilder::label(L"Gamma")
                                .relativeWidth(0.4f)
                                .relativeHeight(1.0f)
                            )
                            .child(
                                LayoutBuilder::widget(new Widgets::Slider<float>())
                                    .apply<Widgets::Slider<float>>(
                                        [this](Widgets::Slider<float>* c)
                                        {
                                            c->setRange(0.1f, 5.0f, 0.05f);

                                            // 2. Биндим данные
                                            c->bind(
                                                [this]()
                                                {
                                                    return settings.gamma;
                                                },
                                                [this](float v)
                                                {
                                                    settings.gamma = v;
                                                    // 3. Здесь можно вызвать принудительную
                                                    // перерисовку окна превью
                                                    // modalTexturePreview->update();
                                                }
                                            );
                                        }
                                    )
                                    .relativeWidth(0.6f)
                                    .relativeHeight(1.0f)
                            )
                    )
                    // Filtering
                    .child(
                        LayoutBuilder::hBox()
                            .relativeWidth(1.0f)
                            .absoluteHeight(30)
                            .child(
                                LayoutBuilder::label(L"Filtering")
                                    .relativeWidth(0.4f)
                                    .relativeHeight(1.0f)
                            )
                            .child(
                                LayoutBuilder::widget(new Widgets::ComboBox())
                                    .relativeWidth(0.6f)
                                    .relativeHeight(1.0f)
                                    .apply<Widgets::ComboBox>(
                                        [&](Widgets::ComboBox* c)
                                        {
                                            c->addItem({L"Point", 0});
                                            c->addItem({L"Bilinear", 1});
                                            c->addItem({L"Trilinear", 2});
                                        }
                                    )
                            )
                    )
            )

            // Пружина (занимает всё оставшееся свободное место, выталкивая кнопки вниз)
            .child(LayoutBuilder::spacer().relativeWidth(1.0f).relativeHeight(1.0f))

            // --- СЕКЦИЯ: КНОПКИ ДЕЙСТВИЙ ---
            .child(
                LayoutBuilder::hBox()
                    .relativeWidth(1.0f)
                    .absoluteHeight(40)
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"REIMPORT")
                            .relativeWidth(0.5f)
                            .relativeHeight(1.0f)
                    )
                    .child(
                        LayoutBuilder::widget(new Widgets::Button())
                            .text(L"EXPORT...")
                            .relativeWidth(0.5f)
                            .relativeHeight(1.0f)
                    )
            )
            .build();


       return inspectorUi;
}

void TextureEditor::setTargetTexture(nb::Renderer::Texture* tex) {
    settings.source = tex;
}

void TextureEditor::onRender()
{
    //if (settings.source) {
        engine->getRenderer()->blitToWindow(sharedContext, settings);
    //}
}

void TextureEditor::show()
{
    textureEditorWindow->show();
    previewWindow->show();
    inspectorWindow->show();
}

void TextureEditor::setVisible(bool visible)
{
}
