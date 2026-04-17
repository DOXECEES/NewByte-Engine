
#include <Win32Window/Win32ModalWindow.hpp>
#include <LayoutBuilder.hpp>
#include <functional>

#include <Reflection/Reflection.hpp>
#include <Error/ErrorManager.hpp>

#include <Widgets/Label.hpp>
#include <Widgets/Button.hpp>
#include <Widgets/SpinBox.hpp>
#include <Common/StringUtils.hpp>

class PrimitiveCreationDialog
{
public:
    using OnCreateCallback = std::function<void(void* data, nb::Reflect::TypeInfo* typeInfo)>;

    PrimitiveCreationDialog(
        WindowInterface::IWindow*           parent,
        const std::string& typeName,
        OnCreateCallback   callback
    )
        : m_callback(std::move(callback)) 
    {
        m_typeInfo = nb::Reflect::TypeRegistry::instance().get(typeName);

        if (!m_typeInfo)
        {
            nb::Error::ErrorManager::instance()
                .report(nb::Error::Type::FATAL, "Reflection type not found")
                .with("typeName", typeName);
            return;
        }

        m_window =
            std::make_shared<Win32Window::ModalWindow>(NbSize<int>{350, 400}, parent);
        m_window->setTitle(L"Configure Primitive");

        m_data = malloc(m_typeInfo->size);
        memset(m_data, 0, m_typeInfo->size); 

        initializeDefaults(typeName);
    }

    ~PrimitiveCreationDialog()
    {
        if (m_data)
        {
            free(m_data);
        }
    }

    void show()
    {
        using namespace nbui;

        auto builder = LayoutBuilder::vBox().style(
            [](auto& s)
            {
                s.color   = {30, 30, 30};
                s.padding = {15, 15, 15, 15};
            }
        );

        // 1. Заголовок (высота 30)
        builder = std::move(builder).child(
            LayoutBuilder::widget(new Widgets::Label(L"Primitive Configuration"))
                .absoluteHeight(30.0f)
                .relativeWidth(1.0f)
        );

        builder = std::move(builder).child(LayoutBuilder::spacerAbsolute(0.0f, 10.0f));

        for (auto& field : m_typeInfo->fields)
        {
            builder = std::move(builder).child(
                LayoutBuilder::widget(new Widgets::Label(nb::Utils::toWString(field.name)))
                    .absoluteHeight(20.0f)
                    .relativeWidth(1.0f)
            );

            void* fieldPtr = (uint8_t*)m_data + field.offset;

            if (strcmp(field.type->name, "float") == 0)
            {
                float currentVal = *static_cast<float*>(fieldPtr);
                builder          = std::move(builder).child(
                    LayoutBuilder::widget(new Widgets::FloatSpinBox(currentVal))
                        .absoluteHeight(30.0f)
                        .relativeWidth(1.0f)
                        .onEvent(
                            &Widgets::FloatSpinBox::onValueChanged,
                            [fieldPtr](float val)
                            {
                                *static_cast<float*>(fieldPtr) = val;
                            }
                        )
                );
            }
            else if (
                strcmp(field.type->name, "int") == 0 || strcmp(field.type->name, "uint32_t") == 0
            )
            {
                float currentVal = (float)(*static_cast<int*>(fieldPtr));
                builder          = std::move(builder).child(
                    LayoutBuilder::widget(new Widgets::FloatSpinBox(currentVal))
                        .absoluteHeight(30.0f)
                        .relativeWidth(1.0f)
                        .onEvent(
                            &Widgets::FloatSpinBox::onValueChanged,
                            [fieldPtr, &field](float val)
                            {
                                if (strcmp(field.type->name, "int") == 0)
                                {
                                    *static_cast<int*>(fieldPtr) = (int)val;
                                }
                                else
                                {
                                    *static_cast<uint32_t*>(fieldPtr) = (uint32_t)val;
                                }
                            }
                        )
                );
            }

            builder = std::move(builder).child(LayoutBuilder::spacerAbsolute(0, 5));
        }

        builder = std::move(builder).child(LayoutBuilder::spacerAbsolute(0, 20));

        builder = std::move(builder).child(
            LayoutBuilder::hBox()
                .absoluteHeight(35.0f)
                .relativeWidth(1.0f)
                .child(
                    LayoutBuilder::widget(new Widgets::Button())
                        .relativeWidth(0.5f)
                        .relativeHeight(1.0f)
                        .text(L"Create")
                        .onEvent(
                            &Widgets::Button::onReleasedSignal,
                            [this]()
                            {
                                if (m_callback)
                                {
                                    m_callback(m_data, m_typeInfo);
                                }
                                close();
                            }
                        )
                )
                .child(LayoutBuilder::spacerAbsolute(10, 0))
                .child(
                    LayoutBuilder::widget(new Widgets::Button())
                        .relativeWidth(0.5f)
                        .relativeHeight(1.0f)
                        .text(L"Cancel")
                        .onEvent(
                            &Widgets::Button::onReleasedSignal,
                            [this]()
                            {
                                close();
                            }
                        )
                )
        );

        m_window->getLayoutRoot()->addChild(std::move(builder).build());
        m_window->show();
    }

private:
    void initializeDefaults(const std::string& typeName)
    {
        // Хак, так как в TypeInfo пока нет конструктора.
        // Проставляем руками базовые значения, если память только что выделена.
        //if (typeName == "SphereParams")
        //{
        //    auto* p = static_cast<nb::Renderer::SphereParams*>(m_data);
        //    //*p      = nb::Renderer::SphereParams(); // Вызов конструктора со значениями по умолчанию
        //}
        //else if (typeName == "CubeParams")
        //{
        //    auto* p = static_cast<nb::Renderer::CubeParams*>(m_data);
        //    //*p      = nb::Renderer::CubeParams();
        //}
    }

    void close()
    {
        SendMessage(m_window->getHandle().as<HWND>(), WM_CLOSE, 0, 0);
    }

    std::shared_ptr<Win32Window::ModalWindow> m_window;
    nb::Reflect::TypeInfo*                        m_typeInfo = nullptr;
    void*                                         m_data     = nullptr;
    OnCreateCallback                              m_callback;
};