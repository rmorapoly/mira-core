# This file holds project identity metadata
# after changing the variables
# qmake must be run manually and the project rebuilt
# for the changes to take effect in the code.

VERSION_MAJOR       = 1
VERSION_MINOR       = 0
VERSION_PATCH       = 0
VERSION_STRING      = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}
# Use \\\x20 as space charachter
PROJECT_NAME        = Verna\\\x20Software\\\x20Guidelines
ORGANIZATION_NAME   = Novin\\\x20Pardazesh\\\x20Verna\\\x20Sanat\\\x20Hooman
ORGANIZATION_DOMAIN = VernaSanat.ir

CONFIG += HAS_VERSION_INFO
CONFIG += HAS_VCS_INFO

DEFINES += VERSION_MAJOR=$$VERSION_MAJOR \
           VERSION_MINOR=$$VERSION_MINOR \
           VERSION_PATCH=$$VERSION_PATCH \
           VERSION_STRING=\\\"$$VERSION_STRING\\\" \
           PROJECT_NAME=\\\"$$PROJECT_NAME\\\" \
           ORGANIZATION_NAME=\\\"$$ORGANIZATION_NAME\\\" \
           ORGANIZATION_DOMAIN=\\\"$$ORGANIZATION_DOMAIN\\\" \
