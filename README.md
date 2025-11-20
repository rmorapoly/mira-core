# Mira Core

Mira is a lightweight C++/Qt framework for structured message handling, object registration, and debugging/logging. It provides centralized management of messages, object lifetimes, and logging sinks.

---

## Features

- **Message handling**: Route messages (`Info`, `Debug`, `Warning`, `Critical`, `Fatal`) to multiple sinks (console, file, network, or custom handlers).
- **Object registration**: Automatically register and track objects using `ObjectRegistry`.
- **Singleton Core**: Central access point to `MessageManager` and `ObjectRegistry`.
- **Convenient macros**: Quick logging via `miraInfo()`, `miraDebug()`, etc.
- **Namespace-safe class name handling** via `__CLASS_NAME__` and `STRINGIFY()` macros.

---

## Quick Start

### 1. Core Singleton

Access the central core instance:

```cpp
#include "MiraCore.h"
using namespace Mira;

auto* core = Core::getInstance();
```

### 2. Logging Messages

You can log messages with different severities using the helper macros:

```cpp
miraInfo() << "Info message";
miraDebug() << "Debug message";
miraWarning() << "Warning message";
miraCritical() << "Critical message";
miraFatal() << "Fatal message"; // will terminate program
```

These macros create a temporary `MessageHelper` object and send the message to `MessageManager`.

### 3. Handling Messages

Connect to `MessageManager::reported` to catch messages for logging, file writing, or custom handling:

```cpp
connect(&core->getMessageManager(), &MessageManager::reported,
        [](const Message& msg){
            /// Handle msg
        }, Qt::DirectConnection);
```

### 4. Object Registration

All objects derived from `Mira::Object` can automatically register in the `ObjectRegistry`:

```cpp
class MyObject : public Mira::Object {
public:
    MyObject() MIRA_DECLARE_OBJECT;
};

MyObject obj;

int total = core->getObjectRegistery().count("MyObject");
auto retrieved = core->getObjectRegistery().get("MyObject");
```
