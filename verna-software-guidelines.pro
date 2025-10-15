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

#QMAKE_LFLAGS += -fuse-ld=lld
#QMAKE_LFLAGS = -fuse-ld=gold

win32 {
    DEFINES += WINDOWS_BUILD
}
unix {
    DEFINES += UNIX_BUILD
}

#CONFIG -= app_bundle
#CONFIG += console

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#CONFIG(debug, debug|release ) {
#    # debug
#    OBJECTS_DIR = debug
#    MOC_DIR = debug
#} else {
#    # release
#    OBJECTS_DIR = release
#    MOC_DIR = release
#}

include($$PWD/src/cpp/cpp.pri)

include($$PWD/src/c/c.pri)

include($$PWD/src/qml/qml.pri)

include($$PWD/src/js/js.pri)

include($$PWD/submodule/submodule.pri)

include($$PWD/version.pri)

allFilesFolders = $$files($$PWD/src/*, true))
folders = src
for (item, allFilesFolders): contains(item, [^.]*) : folders += $$item

INCLUDEPATH += $$folders

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32: DEFINES += _GLIBCXX_TXN_SAFE_DYN="" _GLIBCXX_NOTHROW="" # For compatibility to GCC

TRANSLATIONS += \
    verna-software-guidelines_en_US.ts

win32: RC_ICONS += icon.ico
