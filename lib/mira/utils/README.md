# mira-utils

Vendored snapshot of the reusable Qt/C++ utilities from the `../utils` module.

`libMiraUtils.so.1.0.0` and the headers in `include/` are built from
`../utils/src/cpp`. When the shared sources change, rebuild the library there
and refresh this directory.

## Headers

All headers live under `include/mira/utils/` and are included as
`mira/utils/<Header>.h`.

| Header | Contents |
| --- | --- |
| `Macros.h` | `CHECK_AND_RETURN_IF_NOT`, `RETURN_IF_NOT`, `STRINGIFY`, `__CLASS_NAME__`, `REGISTER_QT_TEST`, `RUN_QT_TESTS` |
| `Meta.h` | `Mira::Utils::className<T>()`, `toStr()`, `demangleClassName()` |
| `Singleton.h` | thread-safe CRTP singleton base `Mira::Utils::Singleton<T>` |
| `TestRegistry.h` | Qt test registry `Mira::Utils::TestRegistry` |
| `ContainerUtils.h` | Conversions between compatible Qt container and value types |
| `EnumUtils.h` | Name and value lookups for Qt-registered enumerations |
| `ByteSwap.h` | Byte-order conversion helpers |
| `OwnedBy.h` | Ownership assertion base class |
| `SettingsJsonFormat.h` | Reading and writing values as JSON |

## Usage

`lib/lib.pri` includes `mira-utils.pri`, which puts `include/` on the include
path, links `libMiraUtils`, and adds this directory to the Unix runtime search
path. No extra configuration is needed:

```cpp
#include "mira/utils/Meta.h"
#include "mira/utils/Macros.h"
#include "mira/utils/Singleton.h"

using namespace Mira::Utils;

QString name = className<MyClass>();
QString text = STRINGIFY(MyIdentifier);
Manager* manager = Manager::getInstance();
```

`Singleton<T>` creation is thread-safe. Destroying a singleton concurrently
with `getInstance()` is not supported; synchronize explicit deletion with all
readers. Destruction clears the instance pointer to allow recreation.
