TEMPLATE = app

QT +=   core           \
        testlib        \
        gui            \
        qml            \
        quick          \
        quickcontrols2 \
        charts         \
        positioning    \
        location       \
        network        \
        widgets        \
        svg            \
        xml            \
        printsupport   \
        concurrent

CONFIG +=   qt                 \
            warn_on            \
            depend_includepath \
            testcase

win32 {
    DEFINES += WINDOWS_BUILD
}
unix {
    DEFINES += UNIX_BUILD
}

#QMAKE_LFLAGS += -fuse-ld=lld
#QMAKE_LFLAGS = -fuse-ld=gold

#CONFIG(DEVELOPMENT){
#   DEFINES += DEVELOPMENT
#}else{
##    QMAKE_CXXFLAGS += -flto
##    QMAKE_LFLAGS += -flto
#}

CONFIG(SANITIZER){
    CONFIG += sanitizer
#    QMAKE_CXXFLAGS += -fno-omit-frame-pointer
    QMAKE_CXXFLAGS += -fsanitize=address
#    QMAKE_CXXFLAGS += -fsanitize=thread
    QMAKE_CXXFLAGS += -fsanitize=leak
    QMAKE_CXXFLAGS += -fsanitize=undefined
    QMAKE_CXXFLAGS += -fsanitize=return
    QMAKE_CXXFLAGS += -fsanitize=bounds
#    QMAKE_CXXFLAGS += -fsanitize=object-size
    LIBS += -lasan -lubsan
}

CONFIG(BUILD_WITH_UNRESOLVED){
    LIBS += -Wl,--as-needed -Wl,--unresolved-symbols=ignore-all
}

allFilesFolders = $$files($$PWD/src/*, true))
folders = src
for (item, allFilesFolders): contains(item, [^.]*) : folders += $$item

INCLUDEPATH += $$folders

include($$PWD/src/src.pri)

include($$PWD/lib/lib.pri)

include($$PWD/submodule/submodule.pri)

include($$PWD/version.pri)

INCLUDEPATH = $$unique(INCLUDEPATH)

RESOURCES += $$PWD/img/img.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: DEFINES += _GLIBCXX_TXN_SAFE_DYN="" _GLIBCXX_NOTHROW="" # For compatibility to GCC

TRANSLATIONS += \
    mira-base_en_US.ts

DISTFILES += \
    README.md \
    $$files($$PWD/doc/*.md, true)

#win32: RC_ICONS += icon.ico










