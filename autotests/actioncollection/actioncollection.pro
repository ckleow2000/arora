TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .

include(../autotests.pri)

# Input
SOURCES = tst_actioncollection.cpp actioncollection.cpp
HEADERS = actioncollection.h
