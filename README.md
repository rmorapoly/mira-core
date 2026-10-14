# Mira Core

Mira is a lightweight C++/Qt framework for structured message handling, object registration, and debugging/logging. It provides centralized management of messages, object lifetimes, and logging sinks.

This repository is the `core` module of the Mira family. Its entry point,
`Mira::Root`, lives in the root `Mira` namespace, while the reusable components
live under `Mira::Core`.

---

## Features

- **Logging on top of Qt**: messages go through Qt's own `qDebug()/qInfo()/...`
  pipeline, so `qSetMessagePattern()` and `qInstallMessageHandler()` keep
  working. Mira's message handler adds the originating object's context through
  the pattern tokens `%{objectName}`, `%{objectInstanceId}`,
  `%{objectInstanceName}` and `%{object}` (see `Mira::Core::setLogPattern()`).
- **Object registration**: automatically register and track objects using
  `Mira::Core::ObjectRegistry`.
- **Singleton entry point**: access everything through `Mira::Root`.
- **Convenient macros**: quick logging via `miraInfo()`, `miraDebug()`, etc.
- **RTTI-free class names** via `Mira::className<T>()` and `__CLASS_NAME__`.

---

## Quick Start

### 1. Core Singleton

Access the central core instance:

```cpp
#include "mira/Root.h"

auto* root = Mira::Root::getInstance();
auto& registry = root->registry();
```

### 2. Logging Messages

Use the helper macros, either in a class deriving from `Mira::Core::Object` or
anywhere else (e.g. `main()`). Inside an `Object` member the message is
attributed to `this`; elsewhere it simply has no object context:

```cpp
#include "mira/core/Object.h"

class MyObject : public Mira::Core::Object {
public:
    MyObject() MIRA_DECLARE_OBJECT {}

    void work() {
        miraInfo() << "Info message";
        miraDebug() << "Debug message";
        miraWarning() << "Warning message";
        miraCritical() << "Critical message";
        miraFatal() << "Fatal message"; // will terminate program
    }
};

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    Mira::Core::installLogHandler();
    mInfo() << "No object context here"; // works in free functions too
    // ...
}
```

Each message is routed through Qt's logging pipeline. The emitting object is
made available to the message handler, which expands the Mira pattern tokens.
`Mira::Core::installLogHandler()` installs the handler and a readable default
pattern; use `Mira::Core::setLogPattern()` to customize it:

```cpp
Mira::Core::setLogPattern("[%{time hh:mm:ss}]%{object} %{message}");
```

The pattern is applied in two ways:

- From a `Mira::Core::Object`, the pattern is applied in full: `%{object}` (and
  `%{objectName}`, `%{objectInstanceId}`, `%{objectInstanceName}`) are filled in
  with the object's context. `%{object}` renders `name:instanceId`, plus
  `#instanceName` when the object has one.
- From any other `QObject` (or plain `qInfo()`/`qDebug()`/etc.), the Mira tokens
  expand to `unknown`, so the object part is clearly marked as absent.

### 3. Object Registration

All objects derived from `Mira::Core::Object` automatically register in the
`ObjectRegistry`:

```cpp
#include "mira/core/Object.h"

class MyObject : public Mira::Core::Object {
public:
    MyObject() MIRA_DECLARE_OBJECT {}
};

MyObject obj;

int total = Mira::Root::getInstance()->registry().count("MyObject");
auto retrieved = Mira::Root::getInstance()->registry().get("MyObject");
```
