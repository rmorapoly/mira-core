# Mira Core

Mira is a lightweight C++/Qt framework for structured message handling, object registration, and debugging/logging. It provides centralized management of messages, object lifetimes, and logging sinks.

This repository is the `core` module of the Mira family. Its entry point,
`Mira::Root`, lives in the root `Mira` namespace, while the reusable components
live under `Mira::Core`.

---

## Features

- **Logging on top of Qt**: messages go through Qt's own `qDebug()/qInfo()/...`
  pipeline, so `qInstallMessageHandler()` and Qt's message patterns keep
  working. Mira fills the originating object's context into the pattern tokens
  `%{objectName}`, `%{objectInstanceId}`, `%{objectInstanceName}` and
  `%{object}`, and a per-message sequence number (row count) into
  `%{messageIndex}`, before Qt renders the message (see
  `Mira::Core::setLogPattern()`).
- **Extra log sinks**: every message can additionally go to your own handler —
  console and sink at the same time (see `Mira::Core::setLogSink()`).
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
anywhere else (e.g. `main()`). They work in every context — member function,
free function, lambda, static member function — and behave exactly like
`qInfo()`/`qDebug()`/...:

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

**Object attribution.** While a `Mira::Core::Object` lives, it is the object in
scope, and every message logged in that time is attributed to it — no matter
where the message is logged from. The object context is therefore available in
free functions and lambdas as well, not only in member functions:

```cpp
MyObject obj;
obj.work();          // attributed to obj
[]() { mInfo() << "also attributed to obj"; }();   // attributed to obj
MyObject::staticWork();                            // attributed to obj
```

An `ObjectScope` attributes a whole block to a chosen object — including an
object other than the innermost one in scope:

```cpp
{
    Mira::Core::ObjectScope scope(obj);
    mInfo() << "attributed to obj";
}
```

Each message is routed through Qt's logging pipeline. Mira fills the Mira
placeholders (`%{object}`, `%{objectName}`, `%{objectInstanceId}`,
`%{objectInstanceName}`) from the emitting object plus `%{messageIndex}`, its
own per-message sequence number — a row count over every message Mira formats.
Qt has no token of its own for this, so Mira consumes it before Qt renders the
line; the token is optional, and a pattern without it keeps the cached,
allocation-free path. The rest is rendered by `qFormatLogMessage()`, so
`%{time}`, `%{type}`, `%{file}`, `%{line}`, `%{function}`, `%{category}`,
`%{pid}`, `%{appname}` and `%{message}` keep Qt's own semantics.

`Mira::Core::installLogHandler()` installs the handler and keeps the pattern
you set — `Mira::Core::defaultPattern()` until you customize it — so a pattern
set before installing is never overwritten. Use `Mira::Core::setLogPattern()`
to choose one, and `Mira::Core::defaultPattern()` /
`Mira::Core::qtDefaultPattern()` to fetch Mira's and Qt's defaults:

```cpp
Mira::Core::setLogPattern("[%{time hh:mm:ss}]%{messageIndex} %{object} %{message}");
// later, back to the default:
Mira::Core::setLogPattern(Mira::Core::defaultPattern());
```

Qt's process-wide pattern is not rewritten for every message: it is only
updated when the emitting object changes (in practice, once per object) —
except for patterns that use `%{messageIndex}`, whose index changes per message
and is therefore re-rendered per message. Whenever a message has no object —
and on `installLogHandler()`, `uninstallLogHandler()` and `setLogPattern()` —
the pattern is left in its placeholder-free form, so `qInstallMessageHandler()`
handlers of your own, QtTest, and Qt's default handler never warn about Mira
placeholders.

Messages are written to the console *and* can be passed to your own extra
handler — e.g. to mirror them into a file, a widget or a network stream:

```cpp
Mira::Core::setLogSink([](QtMsgType, const QMessageLogContext&,
                          const QString& message) {
    myLogFile.write(message + QLatin1Char('\n'));
});
// later: Mira::Core::setLogSink(nullptr);
```

The pattern is applied in two ways:

- From a `Mira::Core::Object`, the Mira placeholders are filled in with the
  object's context. `%{object}` renders `name:instanceId`, plus `#instanceName`
  when the object has one.
- From any other `QObject` (or plain `qInfo()`/`qDebug()`/etc.), the Mira tokens
  expand to `unknown`, so the object part is clearly marked as absent.

Each Mira placeholder also has a conditional form, mirroring Qt's
`%{if-category}...%{endif}`: the block is rendered only when the placeholder is
more than the `unknown` fallback. This keeps patterns clean when a message has
no object:

```cpp
Mira::Core::setLogPattern("%{if-object}[%{object}] %{endif}%{message}");
// object context -> "[MyObj:0] hello"
// no object      -> "hello"
```

Supported conditions are `%{if-object}`, `%{if-objectName}`,
`%{if-objectInstanceId}` and `%{if-objectInstanceName}`, each closed by
`%{endif}`. `if-objectName` and `if-objectInstanceName` are also false when the
matching field is empty. Mira's conditions are evaluated by Mira and their
markers are consumed; Qt's own conditions are left for Qt, so the two can be
mixed in one pattern.

The macros return a real `QDebug`, so the whole `QDebug` API is available and
behaves exactly as it does for `qInfo()`/`qDebug()`/...:

```cpp
miraInfo().nospace() << "Some" << "info" << "message";
miraInfo().noquote() << someString;
```

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

int total = Mira::Root::getInstance()->registry().count("MyObject");  // one name
int all = Mira::Root::getInstance()->registry().countAll();             // every object
auto retrieved = Mira::Root::getInstance()->registry().get("MyObject");
```
