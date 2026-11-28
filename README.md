# Mira Core

Mira is a lightweight C++/Qt framework for structured message handling, object registration, and debugging/logging. It provides centralized management of messages, object lifetimes, and logging sinks.

This repository is the `core` module of the Mira family. Its entry point,
`Mira::Core::Root`, and all reusable components live under `Mira::Core`.

---

## Features

- **Logging on top of Qt**: messages go through Qt's own `qDebug()/qInfo()/...`
  pipeline, so `qInstallMessageHandler()` works and Qt's own message pattern
  (`qSetMessagePattern()` / `QT_MESSAGE_PATTERN`) keeps formatting the line.
  Mira supplies its own part of the line: the originating object's context
  (`%{objectName}`, `%{objectInstanceId}`, `%{objectInstanceName}`,
  `%{object}`) and a per-message sequence number (`%{messageIndex}`), set with
  `Mira::Core::setMessagePattern()`.
- **Extra log sinks**: every message can additionally go to your own handler —
  console and sink at the same time (see `Mira::Core::setLogSink()`). Sinks must
  not throw.
- **Object registration**: automatically register and track objects using
  `Mira::Core::ObjectRegistry`.
- **Singleton entry point**: access everything through `Mira::Core::Root`, which
  uses the thread-safe `Mira::Utils::Singleton<T>` from the vendored utils library.
- **Convenient macros**: quick logging via `miraInfo()`, `miraDebug()`, etc.
- **Compile-time class names** via `Mira::Utils::className<T>()`;
  `__CLASS_NAME__` uses RTTI to name the current object. Both come from the
  vendored `mira-utils` library.

---

## Quick Start

### 1. Core Singleton

Access the central core instance:

```cpp
#include "mira/core/Root.h"

auto* root = Mira::Core::Root::getInstance();
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
    Mira::Core::installMessageHandler();
    mInfo() << "No object context here"; // works in free functions too
    // ...
}
```

**Object attribution.** A message is attributed to the object whose member
function is running. `Mira::Core::Object` declares `miraLogObject()` returning
`this`, which shadows the global `miraLogObject()` returning `nullptr` that the
macros fall back to outside Mira objects. Attribution therefore needs no help
inside a member function, and it stays correct while several objects of one
type are alive:

```cpp
MyObject obj;
MyObject other;
obj.work();          // attributed to obj
other.work();        // attributed to other
mInfo() << "free";   // no object context
```

A lambda that captures `this` keeps the attribution. A captureless lambda and a
static member function have no `this`, so `miraInfo()` does not resolve to the
object member there — capture `this`, or use plain `qInfo()` when no object
should be attributed.

Each message is routed through Qt's logging pipeline, and the line is built
from two independent patterns:

- **Qt's pattern** — `qSetMessagePattern()` / `QT_MESSAGE_PATTERN` — renders
  time, severity, category, file/line, and so on. This is not Mira's job: Mira
  never calls `qSetMessagePattern()`, so it cannot overwrite what you set, and
  no per-message work is spent on Qt's state.
- **Mira's pattern** — `Mira::Core::setMessagePattern()` — supplies the Mira
  placeholders (`%{object}`, `%{objectName}`, `%{objectInstanceId}`,
  `%{objectInstanceName}`), the per-message row count `%{messageIndex}`, and
  the message itself. `%{messageIndex}` is allocated in formatting order; with
  concurrent writers, printed lines may not appear in numeric order.

Mira hands its expansion to Qt as the message text, so a line is
`{Qt pattern}{Mira pattern} message`. A pattern that leaves `%{message}` out is
a prefix and the message is appended after it; an empty pattern prints the
message alone. Qt's tokens are not expanded inside Mira's pattern.

```cpp
qSetMessagePattern("[%{time hh:mm:ss}] [%{type}] %{message}");
Mira::Core::setMessagePattern("[%{messageIndex}] %{object} %{message}");
// -> [12:00:00] [info] [5] MyObj:0 hello
```

You own both patterns: `setMessagePattern()` is the only function that changes
Mira's, and neither `installMessageHandler()` nor `uninstallMessageHandler()`
ever touches it (or Qt's). Until you set one, `messagePattern()` reports
`defaultMessagePattern()`; `qtDefaultMessagePattern()` is Qt's own default for
`qSetMessagePattern()`:

```cpp
Mira::Core::setMessagePattern(Mira::Core::defaultMessagePattern());
qSetMessagePattern(Mira::Core::qtDefaultMessagePattern());
```

Messages are written to the console *and* can be passed to your own extra
handler — e.g. to mirror them into a file, a widget or a network stream:

```cpp
Mira::Core::setLogSink([](QtMsgType, const QMessageLogContext&,
                          const QString& message) {
    myLogFile.write((message + QLatin1Char('\n')).toUtf8());
});
// later: Mira::Core::setLogSink(nullptr);
```

A sink runs inside Qt's C++ message callback and must not throw; the logging
path deliberately has no `try/catch`.

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
Mira::Core::setMessagePattern("%{if-object}[%{object}] %{endif}%{message}");
// object context -> "[MyObj:0] hello"
// no object      -> "hello"
```

Supported conditions are `%{if-object}`, `%{if-objectName}`,
`%{if-objectInstanceId}` and `%{if-objectInstanceName}`, each closed by
`%{endif}`. `if-objectName` and `if-objectInstanceName` are also false when the
matching field is empty. Mira's conditions are evaluated by Mira and their
markers are consumed. Qt's own conditions belong to Qt's pattern; a Qt token
left in Mira's pattern is printed literally.

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

int total = Mira::Core::Root::getInstance()->registry().count("MyObject");  // one name
int all = Mira::Core::Root::getInstance()->registry().countAll();             // every object
auto retrieved = Mira::Core::Root::getInstance()->registry().get("MyObject");
```

---

## Shared utilities

`Meta`, `Singleton`, the stringification macros, and the Qt test registry are
owned by the `../utils` module. Core uses the vendored snapshot in
`lib/mira/utils/` (`libMiraUtils`), wired in by `lib/lib.pri`; there are no
duplicate core implementations of those utilities.

## Documentation

- [Logging](doc/logging.md) — object attribution and its lookup rules.
- [Release notes](doc/release-notes.md) — behaviour changes to be aware of when
  upgrading.

The project follows the shared Mira guidelines known as **Mira Software
Guidelines (MSG)**, including clean-code rules, name conventions, project layout,
and the Qt Test strategy.
