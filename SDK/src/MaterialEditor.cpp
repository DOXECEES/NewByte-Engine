#include "MaterialEditor.hpp"
#include "Resources/TextureAsset.hpp"
#include "Utils.hpp"
#include <Widgets/Slider.hpp>
#include <Widgets/Label.hpp>
#include <Widgets/Button.hpp>

MaterialEditor::MaterialEditor(
    WindowInterface::IWindow* parent,
    nb::Core::Engine* engine,
    nbstl::NonOwningPtr<nb::Resource::MaterialAsset> material
) : engine(engine), targetMaterial(material) 
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

    onRender();
}

std::unique_ptr<NNsLayout::LayoutNode> MaterialEditor::buildInspectorUI()
{
    using namespace nbui;
    auto inspectorVBox = LayoutBuilder::vBox()
        .padding({10, 10, 10, 10})
        .relativeWidth(1.0f)
        .autoHeight();

    // Заголовок секции шейдера
    std::move(inspectorVBox).child(
        LayoutBuilder::label(L"SHADER")
            .fontSize(12).color({150, 150, 150}).absoluteHeight(20)
    );
    
    // Показываем какой шейдер используется
    std::move(inspectorVBox).child(
        LayoutBuilder::label(Utils::toWstring(targetMaterial->getShaderName()))
            .fontSize(14).absoluteHeight(25)
    );

    std::move(inspectorVBox).child(LayoutBuilder::spacerAbsolute(1, 10)); // Разделитель

    // --- ДИНАМИЧЕСКИЕ ПАРАМЕТРЫ ---
    std::move(inspectorVBox).child(
        LayoutBuilder::label(L"PROPERTIES")
            .fontSize(12).color({150, 150, 150}).absoluteHeight(20)
    );

    // Итерируемся по свойствам материала, которые мы загрузили из .material / .shader
    for (auto& [name, prop] : targetMaterial->getProperties()) 
    {
        addPropertyWidget(std::move(inspectorVBox), name, prop);
    }

    // Кнопка сохранения
    std::move(inspectorVBox).child(LayoutBuilder::spacer().relativeHeight(1.0f));
    std::move(inspectorVBox).child(
        LayoutBuilder::widget(new Widgets::Button())
            .text(L"SAVE MATERIAL")
            .absoluteHeight(40).relativeWidth(1.0f)
            .onEvent(&Widgets::IWidget::onReleasedSignal, [this]() {
                //targetMaterial->save(); // Сериализация в JSON
            })
    );

    return std::move(inspectorVBox).build();
}

void MaterialEditor::addPropertyWidget(nbui::LayoutBuilder&& container, const std::string& name, nb::Resource::MaterialProperty& prop)
{
    using namespace nbui;
    std::wstring wName(name.begin(), name.end());

    auto row = LayoutBuilder::hBox().relativeWidth(1.0f).absoluteHeight(30).margin({0, 0, 5, 0});
    std::move(row).child(LayoutBuilder::label(wName).relativeWidth(0.4f));

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
    }
    else if (std::holds_alternative<Ref<nb::Resource::TextureAsset>>(prop.value))
    {
        // Для текстур рисуем кнопку-слот (в идеале тут должен быть Thumbnail)
        auto tex = std::get<Ref<nb::Resource::TextureAsset>>(prop.value);
        std::wstring texName = L"None";

        std::move(row).child(
            LayoutBuilder::widget(new Widgets::Button())
                .text(texName)
                .relativeWidth(0.6f)
                .onEvent(&Widgets::Button::onReleasedSignal, [this, name]() {
                    // Здесь можно открыть диалог выбора текстуры или
                    // активировать режим Drag&Drop
                })
        );
    }

    std::move(container).child(std::move(row));
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