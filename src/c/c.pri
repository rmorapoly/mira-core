CONFIG += c99

QMAKE_CFLAGS += -Wall -Wextra -Wpedantic

INCLUDEPATH += $$PWD/

SOURCES += $$files($$PWD/*.c, true)

HEADERS += $$files($$PWD/*.h, true)
