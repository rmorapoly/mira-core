
equals(TEMPLATE, lib): contains(CONFIG, plugin) { } else {
    DISTFILES += $$files($$PWD/*.qml, true)

    RESOURCES += $$PWD/qml.qrc \
}

include($$PWD/imports/imports.pri)
