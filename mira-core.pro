# Clean repo after changing to lib
TEMPLATE = \
#          app \
          lib

QT +=   core           \
        testlib        \
#        gui            \
#        qml            \
#        quick          \
#        quickcontrols2 \
#        widgets        \
#        charts         \
#        network        \
#        serialport     \
#        positioning    \
#        location       \
#        svg            \
#        xml            \
#        printsupport   \
#        concurrent     \

CONFIG +=   qt                 \
            warn_on            \
#            depend_includepath \
            testcase           \
#            plugin             \    # when trying to create plugin
#            staticlib          \
#            precompile_header

# @info for CONFIG += staticlib (GCC 9 Ubuntu 22)
# Caused to belwoing error:
# Compile outout:
# make: o: No such file or directory
# make: [Makefile:300: repo-name] Error 127 (ignored)
# Application output:
# Failed to start program. Path or permissions wrong?

win32 {
    DEFINES += WINDOWS_BUILD
}
unix {
    DEFINES += UNIX_BUILD
}

DEFINES += REPO_PWD=\\\"$$PWD\\\"
DEFINES += BUILD_OUT_PWD=\\\"$$OUT_PWD\\\"

CONFIG(BUILD_WITH_UNRESOLVED){
    LIBS += -Wl,--as-needed -Wl,--unresolved-symbols=ignore-all
}

include($$PWD/version.pri)

equals(TEMPLATE, lib) {
    message("generating lib")

    # Keep qt config while creating qt lib or plugin lib because of plugin.cpp
    # CONFIG -= qt

    DEFINES += $${PROJECT_NAME_CAPITAL}_LIBRARY

    TARGET = $${PROJECT_NAME_CAMEL}

    CONFIG += shared
} else {
    message("building source codes")

    DEFINES += $${PROJECT_NAME_CAPITAL}_SOURCE
}

include($$PWD/src/src.pri)

include($$PWD/lib/lib.pri)

include($$PWD/submodule/submodule.pri)

INCLUDEPATH = $$unique(INCLUDEPATH)

RESOURCES += $$PWD/img/img.qrc

equals(TEMPLATE, lib): contains(CONFIG, plugin) {
    # Default lib versioning for plugin (should be write for non unix)
    unix: QMAKE_POST_LINK += ln -sf lib$${TARGET}.so lib$${TARGET}.so.$${VERSION_MAJOR};
    unix: QMAKE_POST_LINK += ln -sf lib$${TARGET}.so.$${VERSION_MAJOR} lib$${TARGET}.so.$${VERSION_MAJOR}.$${VERSION_MINOR};
    unix: QMAKE_POST_LINK += ln -sf lib$${TARGET}.so.$${VERSION_MAJOR}.$${VERSION_MINOR} lib$${TARGET}.so.$${VERSION};
} else {
    # Default rules for deployment.
    qnx: target.path = /tmp/$${TARGET}/bin
    else: unix:!android: target.path = /opt/$${TARGET}/bin
    !isEmpty(target.path): INSTALLS += target
}

win32: DEFINES += _GLIBCXX_TXN_SAFE_DYN="" _GLIBCXX_NOTHROW="" # For compatibility to GCC

TRANSLATIONS += \
    mira-base_en_US.ts

DISTFILES += \
    README.md \
    $$files($$PWD/doc/*.md, true)

#win32: RC_ICONS += icon.ico
