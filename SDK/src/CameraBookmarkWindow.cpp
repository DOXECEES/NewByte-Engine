#include "CameraBookmarkWindow.hpp"

#include <Win32Window/Win32ChildWindow.hpp>
#include <Localization/Translation.hpp>

#include <string>
#include <vector>
#include <cwchar>

// Подключение необходимых виджетов
#include "Widgets/IWidget.hpp"
#include "Widgets/Label.hpp"
#include "Widgets/Button.hpp"
#include "Widgets/Spacer.hpp"
#include "Widgets/TableView.hpp" // Подключаем новый виджет

namespace sdk
{
    CameraBookmarkWindow::CameraBookmarkWindow(
        const std::shared_ptr<WindowInterface::IWindow>& parent,
        const CameraBookmarkManager& manager,
        const CameraSettingsController& controller
    ) noexcept
        : manager(manager)
        , controller(controller)
    {
        window = std::make_shared<Win32Window::ChildWindow>(parent.get());
        window->setTitle(
            Localization::Translation::fromKeyToWstring("Ui.Editor.CameraBookMark.Title")
        ); 
        window->getLayoutRoot()->addChild(buildUi().build());
    }

    std::shared_ptr<WindowInterface::IWindow> CameraBookmarkWindow::getWindow() const noexcept
    {
        return window;
    }

    void CameraBookmarkWindow::refreshUi() noexcept
    {
        window->getLayoutRoot()->clearChilds();
        window->getLayoutRoot()->addChild(buildUi().build());
        window->repaint();
    }

    void CameraBookmarkWindow::selectBookmark(const CameraBookmarkModel& bookmark) noexcept
    {
        selectedItem = bookmark;
        refreshUi();
    }

    nbui::LayoutBuilder CameraBookmarkWindow::buildUi() noexcept
    {
        using namespace nbui;

        auto book = manager.getBookmarks();
        
        std::vector<CameraBookmarkModel> mockBookmarks;
        int counter = 0;

        std::for_each(book.begin(), book.end(), [&](const CameraBookmark& bookmark)
        {
            std::wstring bookmarkName = L"bookmark.name_" + std::to_wstring(counter);

            mockBookmarks.push_back({
                bookmarkName,
                100.0f,
                bookmark.position.x,
                bookmark.position.y,
                bookmark.position.z,
                bookmark.direction.x,
                bookmark.direction.y,
                bookmark.direction.z,
                1.5f,
                bookmark.isValid
            });

            counter++; 
        });

        // Инициализируем выбранный по умолчанию элемент
        if (selectedItem.name.empty() && !mockBookmarks.empty())
        {
            selectedItem = mockBookmarks.front();
        }

        // Вспомогательные лямбда-функции для форматирования строк
        auto formatCoords = [](float x, float y, float z) -> std::wstring {
            wchar_t buf[64];
            std::swprintf(buf, 64, L"%.1f, %.1f, %.1f", x, y, z);
            return std::wstring(buf);
        };

        auto formatTime = [](float seconds) -> std::wstring {
            if (seconds <= 0.0f) return L"Мгновенно";
            wchar_t buf[16];
            std::swprintf(buf, 16, L"%.1fs", seconds);
            return std::wstring(buf);
        };

        // 1. Создание корневого вертикального контейнера
        auto rootLayout = LayoutBuilder::vBox()
            .relativeWidth(1.0f)
            .relativeHeight(1.0f)
            .background(NbColor{24, 24, 24}) 
            .spacing(0);

        // 2. Строка поиска (Search Bar)
        auto searchBar = LayoutBuilder::hBox()
            .relativeWidth(1.0f)
            .absoluteHeight(28)
            .background(NbColor{33, 33, 33}) 
            .padding(Padding<int>{4, 8, 4, 8})
            .border(1, Border::Style::SOLID, NbColor{53, 53, 53}, Border::Side::BOTTOM)
            .child(LayoutBuilder::label(L"🔍")
                .absoluteWidth(16)
                .relativeHeight(1.0f) 
                .color(NbColor{128, 128, 128}))
            .child(LayoutBuilder::label(L"Поиск закладок...")
                .relativeWidth(1.0f)
                .relativeHeight(1.0f) 
                .color(NbColor{128, 128, 128}));

        rootLayout = std::move(rootLayout).child(std::move(searchBar));

        // 3. Таблица (TableView) вместо ScrollBox с ручной версткой строк
        auto tableView = LayoutBuilder::tableView()
            .relativeWidth(1.0f)
            .relativeHeight(0.60f)
            .background(NbColor{33, 33, 33})
            .apply<Widgets::TableView>([&](Widgets::TableView* tv) {
                tv->clear();
                
                // Настройка структуры колонок
                tv->setColumns({
                    { L"НАЗВАНИЕ", 0.30f },
                    { L"XYZ", 0.28f },
                    { L"DIR", 0.28f },
                    { L"ВРЕМЯ", 0.14f }
                });

                // Добавление строк данных в виджет
                for (size_t i = 0; i < mockBookmarks.size(); ++i)
                {
                    const auto& bookmark = mockBookmarks[i];
                    bool isCurrentSelected = (bookmark.name == selectedItem.name);

                    if (isCurrentSelected)
                    {
                        tv->setSelectedIndex(i);
                    }

                    tv->addRow({
                        { bookmark.name, NbColor{255, 255, 255} },
                        { formatCoords(bookmark.x, bookmark.y, bookmark.z), NbColor{220, 220, 220} },
                        { formatCoords(bookmark.yaw, bookmark.pitch, bookmark.roll), NbColor{133, 133, 133} },
                        { formatTime(bookmark.transitionTime), NbColor{220, 220, 220} }
                    }, isCurrentSelected);
                }
            })
            // Вешаем обработку клика на сигнал таблицы
            .onEvent(&Widgets::TableView::onRowClickSignal, [this, mockBookmarks](int rowIndex) {
                if (rowIndex >= 0 && rowIndex < static_cast<int>(mockBookmarks.size()))
                {
                    this->selectBookmark(mockBookmarks[rowIndex]);
                }
            });

        rootLayout = std::move(rootLayout).child(std::move(tableView));

        // 4. Панель свойств (динамически отображает выбранную закладку)
        auto propertiesSection = LayoutBuilder::section(L"СВОЙСТВА ВЫБРАННОЙ ЗАКЛАДКИ", false)
            .relativeWidth(1.0f)
            .relativeHeight(0.40f) 
            .background(NbColor{24, 24, 24})
            .padding(Padding<int>{8, 8, 8, 8});

        // Строка: Название (использует имя выбранного элемента)
        auto nameRow = LayoutBuilder::hBox()
            .relativeWidth(1.0f)
            .absoluteHeight(32)
            .spacing(6)
            .child(LayoutBuilder::label(L"Название:").absoluteWidth(65).relativeHeight(1.0f).color(NbColor{133, 133, 133}))
            .child(LayoutBuilder::label(selectedItem.name)
                .relativeWidth(1.0f)
                .relativeHeight(1.0f) 
                .padding(Padding<int>{3, 6, 3, 6})
                .background(NbColor{33, 33, 33})
                .border(1, Border::Style::SOLID, NbColor{51, 51, 51}));

        propertiesSection = std::move(propertiesSection).child(std::move(nameRow));

        // Строка: Координаты Позиции (XYZ)
        auto posRow = LayoutBuilder::hBox()
            .relativeWidth(1.0f)
            .absoluteHeight(32)
            .spacing(6)
            .child(LayoutBuilder::label(L"Позиция:").absoluteWidth(65).relativeHeight(1.0f).color(NbColor{133, 133, 133}))
            
            // Ось X (Красный)
            .child(LayoutBuilder::hBox()
                .relativeWidth(0.33f)
                .relativeHeight(1.0f) 
                .background(NbColor{33, 33, 33})
                .border(1, Border::Style::SOLID, NbColor{51, 51, 51})
                .child(LayoutBuilder::label(L"X").background(NbColor{198, 59, 59}).absoluteWidth(14).relativeHeight(1.0f).textAlignment({.textAlignment = TextAlignment::CENTER}).color(NbColor{255, 255, 255}))
                .child(LayoutBuilder::floatSlider().relativeWidth(1.0f).relativeHeight(1.0f).textAlignment({.textAlignment = TextAlignment::RIGHT, .gap = 5}).color(NbColor{220, 220, 220})))
            
            // Ось Y (Зеленый)
            .child(LayoutBuilder::hBox()
                .relativeWidth(0.33f)
                .relativeHeight(1.0f) 
                .background(NbColor{33, 33, 33})
                .border(1, Border::Style::SOLID, NbColor{51, 51, 51})
                .child(LayoutBuilder::label(L"Y").background(NbColor{59, 198, 59}).absoluteWidth(14).relativeHeight(1.0f).textAlignment({.textAlignment = TextAlignment::CENTER}).color(NbColor{255, 255, 255}))
                .child(LayoutBuilder::floatSlider().relativeWidth(1.0f).relativeHeight(1.0f).textAlignment({.textAlignment = TextAlignment::RIGHT, .gap = 5}).color(NbColor{220, 220, 220})))
            
            // Ось Z (Синий)
            .child(LayoutBuilder::hBox()
                .relativeWidth(0.33f)
                .relativeHeight(1.0f) 
                .background(NbColor{33, 33, 33})
                .border(1, Border::Style::SOLID, NbColor{51, 51, 51})
                .child(LayoutBuilder::label(L"Z").background(NbColor{59, 125, 198}).absoluteWidth(14).relativeHeight(1.0f).textAlignment({.textAlignment = TextAlignment::CENTER}).color(NbColor{255, 255, 255}))
                .child(LayoutBuilder::floatSlider().relativeWidth(1.0f).relativeHeight(1.0f).textAlignment({.textAlignment = TextAlignment::RIGHT, .gap = 5}).color(NbColor{220, 220, 220})));

        propertiesSection = std::move(propertiesSection).child(std::move(posRow));

        // Строка: FOV и Время перехода
        std::wstring fovText = std::to_wstring(static_cast<int>(selectedItem.fov)) + L"°";

        auto extraRow = LayoutBuilder::hBox()
            .relativeWidth(1.0f)
            .absoluteHeight(32)
            .spacing(6)
            .child(LayoutBuilder::label(L"FOV:").absoluteWidth(65).relativeHeight(1.0f).color(NbColor{133, 133, 133}))
            .child(LayoutBuilder::label(fovText)
                .relativeWidth(0.35f)
                .relativeHeight(1.0f) 
                .padding(Padding<int>{3, 6, 3, 6})
                .background(NbColor{33, 33, 33})
                .border(1, Border::Style::SOLID, NbColor{51, 51, 51}))
            .child(LayoutBuilder::label(L"Переход:")
                .absoluteWidth(60)
                .relativeHeight(1.0f) 
                .textAlignment({.textAlignment = TextAlignment::RIGHT})
                .color(NbColor{133, 133, 133}))
            .child(LayoutBuilder::label(formatTime(selectedItem.transitionTime))
                .relativeWidth(0.35f)
                .relativeHeight(1.0f) 
                .padding(Padding<int>{3, 6, 3, 6})
                .background(NbColor{33, 33, 33})
                .border(1, Border::Style::SOLID, NbColor{51, 51, 51}));

        propertiesSection = std::move(propertiesSection).child(std::move(extraRow));

        // Системные кнопки внизу панели свойств
        auto* captureBtn = new Widgets::Button();
        captureBtn->setText(L"Захватить вьюпорт");
        captureBtn->setColor(NbColor{40, 40, 40});

        auto* applyBtn = new Widgets::Button();
        applyBtn->setText(L"Применить");
        applyBtn->setColor(NbColor{31, 56, 92});

        auto actionButtonsRow = LayoutBuilder::hBox()
            .relativeWidth(1.0f)
            .absoluteHeight(32)
            .spacing(6)
            .child(LayoutBuilder::widget(captureBtn).relativeWidth(0.5f).relativeHeight(1.0f))
            .child(LayoutBuilder::widget(applyBtn).relativeWidth(0.5f).relativeHeight(1.0f));

        propertiesSection = std::move(propertiesSection).child(std::move(actionButtonsRow));

        // Добавление настроенной секции свойств в корень макета
        rootLayout = std::move(rootLayout).child(std::move(propertiesSection));

        auto cameraSettingsSection = LayoutBuilder::section(L"Настройки камеры", false)
            .relativeWidth(1.0f)
            .relativeHeight(0.20f) 
            .background(NbColor{24, 24, 24})
            .padding(Padding<int>{8, 8, 8, 8})
            .child(LayoutBuilder::hBox()
                .relativeWidth(1.0f)
                .absoluteHeight(32)
                .spacing(6)
                .child(LayoutBuilder::label(L"Базовая скорость:").relativeWidth(0.5f).relativeHeight(0.4f).color(NbColor{133, 133, 133}))
                .child(LayoutBuilder::label(L"Множитель (Ctrl)").relativeWidth(0.5f).relativeHeight(0.4f).color(NbColor{133, 133, 133}))
            )
            .child(LayoutBuilder::hBox()
                .relativeWidth(1.0f)
                .absoluteHeight(32)
                .spacing(6)
                .child(
                    LayoutBuilder::floatSlider()
                    .relativeWidth(0.5f)
                    .relativeHeight(1.0f)
                    .textAlignment({.textAlignment = TextAlignment::RIGHT, .gap = 5})
                    .color(NbColor{220, 220, 220})
                    .apply<Widgets::Slider<float>>([this](Widgets::Slider<float>* slider)
                    {
                        slider->setRange(0.0f, 100.0f, 0.5f);
                        slider->setValue(1.0f);
                        slider->bind(
                            [this]() -> float { return settings.speed; },
                            [this](float value) { settings.speed = value; controller.update(settings);}
                        );

                    })
                )
                .child(
                    LayoutBuilder::floatSlider()
                    .relativeWidth(0.5f)
                    .relativeHeight(1.0f)
                    .textAlignment({.textAlignment = TextAlignment::RIGHT, .gap = 5})
                    .color(NbColor{220, 220, 220})
                    .apply<Widgets::Slider<float>>([this](Widgets::Slider<float>* slider)
                    {
                        slider->setRange(0.0f, 100.0f, 0.5f);
                        slider->setValue(1.0f);
                        slider->bind(
                            [this]() -> float { return settings.multiplier; },
                            [this](float value) { settings.multiplier = value; controller.update(settings); }
                        );

                    })
            ));

        rootLayout = std::move(rootLayout).child(std::move(cameraSettingsSection));


        return rootLayout;
    }
}