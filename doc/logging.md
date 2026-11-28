# Logging

## Object attribution and name lookup

The `miraInfo()` family of macros calls `miraLogObject()` **unqualified**. Inside
a member function of a `Mira::Core::Object`, ordinary C++ name lookup finds the
member `Mira::Core::Object::miraLogObject()` first, so the message is attributed
to the object whose member function is running. Everywhere else the global
fallback `::miraLogObject()` returns `nullptr`, so the message has no object
context.

This lookup rule has a caveat worth knowing:

> Any other declaration of `miraLogObject()` visible at the call site hides the
> global fallback. A function or variable with that name in an enclosing
> namespace, a free function found by ordinary lookup, or a member of an
> unrelated class can change which overload is selected — or make the macro fail
> to compile. The fallback is deliberately unqualified so the member can win;
> it cannot also be protected from every other declaration.

Examples that lose attribution:

```cpp
namespace app {
void miraLogObject();          // hides ::miraLogObject()
void log() { miraInfo() << "x"; }
}

struct Helper {
    void miraLogObject();      // hides ::miraLogObject() inside Helper
    void log() { miraInfo() << "x"; }
};
```

Static member functions and lambdas that do not capture `this` also have no
implicit object to find. Capture `this`, or use plain `qInfo()` when no object
should be attributed.

If attribution must be explicit, pass the object into a helper that publishes it
with `Mira::Core::LogObjectScope` for the duration of a logging statement. This
also avoids relying on name lookup in uncommon call contexts.

The object pointer published by a logging statement is valid only for that full
expression. A `LogSink` sees the pointer through `Mira::Core::currentLogObject()`
while the handler runs, but must not retain it: the object may be destroyed as
soon as the statement ends. The sink is a no-throw callback: it runs inside Qt's
message handler, where an escaping exception cannot be handled safely.
