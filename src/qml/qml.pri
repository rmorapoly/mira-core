DISTFILES += $$files($$PWD/*.qml, true)

!CONFIG(DEVELOPMENT){
    RESOURCES += $$PWD/qml.qrc
}
