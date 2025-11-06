# This file holds project identity metadata
# after changing the variables
# qmake must be run manually and the project rebuilt
# for the changes to take effect in the code.

VERSION_MAJOR           = 1
VERSION_MINOR           = 0
VERSION_PATCH           = 0
PROJECT_NAME            = Mira Base
ORGANIZATION_NAME       = Mira
ORGANIZATION_DOMAIN     = Mira.ir

# Do not change from this line
# Used \\\x20 as space charachter
VERSION_STRING          = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_PATCH}
PROJECT_NAME_PARTS      = $$split(PROJECT_NAME," ")
PROJECT_NAME            = $$join(PROJECT_NAME_PARTS,\\\\\\x20)
PROJECT_NAME_CAMEL      = $$join(PROJECT_NAME_PARTS,)
PROJECT_NAME_CAPITAL    = $$upper($$join(PROJECT_NAME_PARTS,_))
ORGANIZATION_NAME_PARTS = $$split(ORGANIZATION_NAME," ")
ORGANIZATION_NAME       = $$join(ORGANIZATION_NAME_PARTS,\\\\\\x20)

CONFIG += HAS_VERSION_INFO
CONFIG += HAS_VCS_INFO
VERSION = $$VERSION_STRING

DEFINES += VERSION_MAJOR=$$VERSION_MAJOR \
           VERSION_MINOR=$$VERSION_MINOR \
           VERSION_PATCH=$$VERSION_PATCH \
           VERSION_STRING=\\\"$$VERSION_STRING\\\" \
           PROJECT_NAME=\\\"$$PROJECT_NAME\\\" \
           PROJECT_NAME_CAPITAL=$$PROJECT_NAME_CAPITAL \
           PROJECT_NAME_CAMEL=$$PROJECT_NAME_CAMEL \
           ORGANIZATION_NAME=\\\"$$ORGANIZATION_NAME\\\" \
           ORGANIZATION_DOMAIN=\\\"$$ORGANIZATION_DOMAIN\\\" \
