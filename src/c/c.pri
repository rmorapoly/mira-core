CONFIG += c99

QMAKE_CFLAGS += -Wall

INCLUDEPATH += $$PWD/

SOURCES += $$files($$PWD/*.c, true)

HEADERS += $$files($$PWD/*.h, true)
