# allFilesFolders = $$files($$PWD/*, true)
# folders = src
# for (item, allFilesFolders) {
#     contains(item, [^.]*) {
#         base = $$basename(item)
#         !equals(base, qmldir) {
#             folders += $$item
#         }
#     }
# }

INCLUDEPATH += $$PWD

include($$PWD/cpp/cpp.pri)

#include($$PWD/c/c.pri)

#include($$PWD/qml/qml.pri)

#include($$PWD/js/js.pri)
