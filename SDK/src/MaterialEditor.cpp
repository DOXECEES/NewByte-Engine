#include "MaterialEditor.hpp"
#include "Resources/TextureAsset.hpp"
#include "Utils.hpp"
#include <Widgets/Slider.hpp>
#include <Widgets/Label.hpp>
#include <Widgets/Button.hpp>
#include <Widgets/TextureWidget.hpp>

#include <Localization/Translation.hpp>
#include <Common/StringUtils.hpp>

MaterialEditor::MaterialEditor(
    WindowInterface::IWindow* parent,
    nb::Core::Engine* engine,
    nbstl::NonOwningPtr<nb::Resource::MaterialAsset> material
) 
    : engine(engine)
    , targetMaterial(material) 
{
    modalWindow = std::make_shared<Win32Window::ModalWindow>(NbSize<int>{1200, 800}, parent);
    modalWindow->setTitle(L"Material Editor - ");

    previewWindow = std::make_shared<Win32Window::ChildWindow>(modalWindow.get(), true);
    sharedContext = engine->getRenderer()->createSharedContextForWindow(previewWindow->getHandle().as<HWND>());

    inspectorWindow = std::make_shared<Win32Window::ChildWindow>(modalWindow.get());
    
    handleResize(modalWindow->getClientSize());

    //modalWindow->getLayoutRoot()->addChild(std::move(buildEditorUI()));
    inspectorWindow->getLayoutRoot()->addChild(std::move(buildInspectorUI()));
    
    subscribe(
        previewWindow.get(), &WindowInterface::IWindow::onMouseMove,
        [this](const NbPoint<float>& point)
        {
            static NbPoint<float> lastPoint = point;

            if (previewWindow->isMouseCurrentlyDragging())
            {
                float dx = point.x - lastPoint.x;
                float dy = point.y - lastPoint.y;

                request.x = dx;
                request.y = dy;

                onRender();
            }

            lastPoint = point;
        }
    );
    
    subscribe(
        modalWindow.get(), &Win32Window::ModalWindow::onSizeChanged,
        [this](const NbSize<int>& newSize)
        {
            handleResize(newSize);
        }
    );

    onRender();

}

void MaterialEditor::handleResize(const NbSize<int>& size)
{
    const int previewWidth = static_cast<int>(size.width * Metrics::previewRatio);
    const int contentHeight = size.height - Metrics::toolbarHeight;
    const int topOffset = Metrics::titleBarHeight + Metrics::toolbarHeight;

    const NbRect<int> clientArea = modalWindow->getClientRect();

    previewWindow->setSize({previewWidth, contentHeight});
    previewWindow->setPosition({Metrics::padding, topOffset});

    inspectorWindow->setSize({size.width - previewWidth + Metrics::padding, contentHeight});
    inspectorWindow->setPosition({previewWidth, topOffset});
    inspectorWindow->getLayoutRoot()->addChild(std::move(buildInspectorUI()));

    onRender();
}

std::unique_ptr<NNsLayout::LayoutNode> MaterialEditor::buildInspectorUI()
{
    using namespace Localization;

    using namespace nbui;
    auto inspectorVBox = LayoutBuilder::vBox()
        .padding({10, 10, 10, 10})
        .relativeWidth(1.0f)
        .relativeHeight(1.0f);

    // Заголовок секции шейдера
    std::move(inspectorVBox).child(
            LayoutBuilder::label(Translation::fromKeyToWstring("Ui.MaterialEditor.Shader"))
            .fontSize(12).color({150, 150, 150}).absoluteHeight(20)
    );
    
    // Показываем какой шейдер используется
    std::move(inspectorVBox).child(
        LayoutBuilder::label(Utils::toWstring(targetMaterial->getShaderName()))
            .fontSize(14).absoluteHeight(25)
    );

    std::move(inspectorVBox).child(LayoutBuilder::spacerAbsolute(1, 10)); // Разделитель

    // --- ДИНАМИЧЕСКИЕ ПАРАМЕТРЫ ---
    std::move(inspectorVBox)
        .child(
            LayoutBuilder::label(Translation::fromKeyToWstring("Ui.MaterialEditor.Properties"))
                .fontSize(12)
                .color({150, 150, 150})
                .absoluteHeight(20)
        );

    
    auto textureSection =
        LayoutBuilder::section(Translation::fromKeyToWstring("Ui.MaterialEditor.TextureSection"))
                              .relativeWidth(1.0f)
                              .autoHeight()
                              .style(
                                  [this](auto& s)
                                  {
                                      s.color = {52, 52, 52};
                                  }
                              )
                              .padding({10, 10, 0, 10});
   
    auto valueSection =
        LayoutBuilder::section(Translation::fromKeyToWstring("Ui.MaterialEditor.ParametersSection"))
                              .relativeWidth(1.0f)
                              .autoHeight()
                              
                              .style(
                                  [this](auto& s)
                                  {
                                      s.color = {52, 52, 52};
                                  }
                              )
                              .padding({10, 10, 0, 10});    



    for (auto& [name, prop] : targetMaterial->getProperties()) 
    {
        addPropertyWidget(
            std::move(textureSection), 
            std::move(valueSection),
            Translation::fromKey(name),
            prop
        );
    }

    std::move(inspectorVBox).child(std::move(textureSection)).child(std::move(valueSection));

    // Кнопка сохранения
    std::move(inspectorVBox).child(LayoutBuilder::spacer().relativeHeight(1.0f));
   
    std::move(inspectorVBox).child(
        LayoutBuilder::hBox()
                .absoluteHeight(40)
        .relativeWidth(1.0f)
        .child(
                LayoutBuilder::widget(new Widgets::Button())
                        .text(Translation::fromKeyToWstring("Ui.MaterialEditor.SaveMaterial"))
                    .absoluteHeight(40)
                    .relativeWidth(0.5f)
                    .onEvent(
                        &Widgets::IWidget::onReleasedSignal,
                        [this]()
                        {
                            onSave();
                        }
                    )
           )
        .child(
                    LayoutBuilder::widget(new Widgets::Button())
                        .text(Translation::fromKeyToWstring("Ui.MaterialEditor.Exit"))
                        .absoluteHeight(40)
                        .relativeWidth(0.5f)
                        .onEvent(
                            &Widgets::IWidget::onReleasedSignal,
                            [this]()
                            {
                                onClose();
                            }
                        )
        )

        
    );

    return std::move(inspectorVBox).build();
}

void MaterialEditor::addPropertyWidget(
    nbui::LayoutBuilder&& container,
    nbui::LayoutBuilder&& valueContainer,
    const std::string&              name,
    nb::Resource::MaterialProperty& prop
)
{
    using namespace nbui;
    std::wstring wName = Utils::toWstring(name);

    auto row = LayoutBuilder::hBox().relativeWidth(1.0f).absoluteHeight(30).margin({5, 5, 5, 5});
    std::move(row).child(
        LayoutBuilder::label(wName).relativeWidth(0.4f).textAlignment(
            TextFormatAlignment{
                .textAlignment      = TextAlignment::LEFT,
                .paragraphAlignment = ParagraphAlignment::CENTER,
            }
            )
            
    );

    // Проверяем тип свойства (std::variant)
    if (std::holds_alternative<float>(prop.value)) 
    {
        std::move(row).child(
            LayoutBuilder::widget(new Widgets::Slider<float>())
                .relativeWidth(0.6f)
                .apply<Widgets::Slider<float>>([this, &prop](auto* s) {
                    s->setRange(0.0f, 1.0f, 0.01f);
                    s->bind(
                        [&]() { return std::get<float>(prop.value); },
                        [this, &prop](float v) { 
                            prop.value = v; 
                            onRender(); // Перерисовываем превью при изменении
                        }
                    );
                })
        );

        std::move(valueContainer).child(std::move(row));

    }
    else if (std::holds_alternative<Ref<nb::Resource::TextureAsset>>(prop.value))
    {

        std::move(row).absoluteHeight(100);
        // Для текстур рисуем кнопку-слот (в идеале тут должен быть Thumbnail)
        auto tex = std::get<Ref<nb::Resource::TextureAsset>>(prop.value);
        //std::wstring texName = Localization::Translation::fromKeyToWstring("Ui.MaterialEditor.None");
        std::wstring texName = Utils::toWstring(tex->getPath());
        std::move(row).child(
            LayoutBuilder::widget(new Widgets::TextureWidget())
                .text(texName)
                .relativeWidth(0.6f)
                .apply<Widgets::TextureWidget>(
                    [tex, texName](Widgets::TextureWidget* w)
                    {
                        std::wstring resolution = std::to_wstring(tex->getWidth()) + L"x" +
                                                 std::to_wstring(tex->getHeight());
                        w->setTexture(texName, resolution);
                    }
                )
                //.onEvent(&Widgets::Button::onReleasedSignal, [this, name]() {
                //    // Здесь можно открыть диалог выбора текстуры или
                //    // активировать режим Drag&Drop
                //})
        );
        std::move(container).child(std::move(row));
    }

}

void MaterialEditor::onSave() noexcept
{
    targetMaterial->updateMetaData();
    onClose();
}

void MaterialEditor::onClose() noexcept
{
    inspectorWindow->close();
    previewWindow->close();
    modalWindow->close();
    onWindowClose.emit();
}

void MaterialEditor::show()
{
    modalWindow->show();
    previewWindow->show();
    inspectorWindow->show();
}

void MaterialEditor::onRender()
{
    request.material = targetMaterial.get();

    if (request.material)
        engine->getRenderer()->renderMaterialPreview(sharedContext, request);
}

MaterialEditor::~MaterialEditor()
{
    engine->getRenderer()->releaseSharedContextForWindow(sharedContext);
}