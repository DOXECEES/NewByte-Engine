# C++ Hpp & Cpp File Generator

## Использование

**Запуск скрипта:**

```bash
python3 file_gen.py <directory> <filename> [--guard_prefix PREFIX] [--project_root ROOT]
```
1. **filename** указывается в CamelCase без расширения. 
2. **directory** чувствителен к регистру.

**Пример:**

```bash
python3 file_gen.py NewByte-UI-Lib/src/Layout vissual
```

## Конфигурация (`file_gen_config.json`)

```json
{
    "project_root": "C:/Repos/Engine/NewByte-Engine",
    "guard_prefix": "NB",
    "folder_prefixes": {
        "Engine": "NB",
        "nbstl": "NBSTL",
        "newbyte-ui-lib": "NBUI"
    },
    "namespaces": {
        "Engine": "nb",
        "nbstl": "nbstl",
        "newbyte-ui-lib": "nbui"
    }
}
```

1. **project_root** – корневая папка проекта, относительно которой строятся пути
2. **guard_prefix** – дефолтный префикс
3. **folder_prefixes** – названия папок, для которых дефолтный префикс заменяется на значение ключа
4. **namespaces** – названия папок, в которых классы будут обернуты в корневой namespace 


## Правила генерации

**Header guard** формируется как:

```
<PREFIX>_SRC_<SUBDIRS>_<FILENAME>_HPP
```
где `<SUBDIRS>` – папки после `src` (если есть).
**Namespace** берётся из конфигурации `namespaces` по первой совпавшей корневой папке.
Если после корневой папки нет `src`, выводится `[WARNING]`.
Классы имеют CamelCase имя файла.
Если файлы уже существуют – они не перезаписываются, выводится `[WARNING]`.

## Структура создаваемых файлов

**Header (.hpp):**

```cpp
#ifndef NBUI_SRC_LAYOUT_VISSUAL_HPP
#define NBUI_SRC_LAYOUT_VISSUAL_HPP

namespace nbui
{
    class Vissual
    {
    public:
        Vissual();
        ~Vissual();
    };
}; // namespace nbui

#endif // NBUI_SRC_LAYOUT_VISSUAL_HPP
```

**Source (.cpp):**

```cpp
#include "vissual.hpp"

namespace nbui
{
    Vissual::Vissual() {}
    Vissual::~Vissual() {}
}; // namespace nbui
```

## Примечания

1. Пока поддерживаются только корневые namespaces. 
2. Файлы не добавляются в CMakeLists.