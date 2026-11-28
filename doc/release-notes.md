# Release notes

## 1.0.0

### Logging patterns are split

`%{time}`, `%{type}`, `%{file}`, `%{category}` and the other Qt placeholders now
belong to Qt's process-wide pattern, set with `qSetMessagePattern()` or
`QT_MESSAGE_PATTERN`. Mira's pattern, set with `Mira::Core::setMessagePattern()`,
contains only Mira's part: `%{messageIndex}`, the `%{object*}` placeholders,
`%{message}` and Mira's `%{if-object*}` conditions.

Mira never calls `qSetMessagePattern()`. A Qt token left in Mira's pattern is
printed literally; move it to Qt's pattern instead. The old single-pattern API
(`setLogPattern()`, `logPattern()`, `defaultPattern()`, `qtDefaultPattern()`) is
renamed to the `...MessagePattern()` forms.

Mira's default pattern no longer contains `%{time}` or `%{type}`. To keep the
old look, set both patterns explicitly:

```cpp
qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] %{message}");
Mira::Core::setMessagePattern(Mira::Core::defaultMessagePattern());
```

`%{messageIndex}` is allocated in formatting order and is never repeated. Lines
written by concurrent threads can therefore appear out of numeric order, but
each complete stderr line is written under the stream lock, so lines never
interleave.

### Log sinks

A `LogSink` is still called after the console write and with the emitting object
still published. It is a no-throw callback: the logging path has no `try/catch`,
so a sink that throws terminates the process. The published object is valid only
for the logging full expression and must not be retained by a sink.

### Threading

`ObjectRegistry` guards its maps and reserved IDs with a mutex, so objects may
be created or destroyed from different threads. Instance IDs are allocated from
the lowest free slot for each name: deleted IDs are reused, and live objects of
one name occupy the dense range `0..count-1`. Concurrent constructors reserve
distinct IDs until registration completes.
`Mira::Utils::Singleton<T>::getInstance()` is thread-safe
for creation; destroying a singleton concurrently with `getInstance()` is still
the caller's responsibility.

### Shared utilities

`Meta`, `Singleton` and the Qt test registry now come from `../utils` and are
linked as the vendored `lib/mira/utils/libMiraUtils` library. The duplicated
core copies were removed; core code uses `Mira::Utils` types and headers.

### Tests

Test suites use a `LogStateGuard` fixture to reset the Qt pattern, Mira pattern,
message handler and log sink before and after every test slot. Tests no longer
depend on execution order or on another suite's global logging state.
