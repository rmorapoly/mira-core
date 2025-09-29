CONFIG += c++1z

QMAKE_CXXFLAGS += -Wall -Wextra -Wpedantic

INCLUDEPATH += $$PWD/

SOURCES += $$files($$PWD/*.cpp, true)

HEADERS += $$files($$PWD/*.h, true)
