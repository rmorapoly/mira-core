CONFIG += c++1z

QMAKE_CXXFLAGS += -Wall

INCLUDEPATH += $$PWD/

SOURCES += $$files($$PWD/*.cpp, true)

HEADERS += $$files($$PWD/*.h, true)
