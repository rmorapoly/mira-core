INCLUDEPATH += $$PWD/

unix: LIBS += -lpthread

LIBS += -L$$PWD/

PRI_FILES = $$files($$PWD/*.pri, true)

PRI_FILES -= $$PWD/lib.pri

for(file, PRI_FILES) {
    include($$file)
}

QML_IMPORT_PATH += $$PWD
QML2_IMPORT_PATH += $$PWD
