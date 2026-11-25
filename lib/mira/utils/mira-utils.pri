INCLUDEPATH += $$PWD/include

HEADERS += $$files($$PWD/*.h, true)

LIBS += -L$$PWD/ -lMiraUtils

unix: QMAKE_RPATHDIR += $$PWD

DISTFILES += \
    $$PWD/README.md
