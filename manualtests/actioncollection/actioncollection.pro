TEMPLATE = app
TARGET =
DEPENDPATH += .
INCLUDEPATH += .

QT += webkit

include(../manualtests.pri)

FORMS += action.ui keyboardshortcutsdialog.ui

HEADERS += actioncollection.h actionmanager.h mainwindow.h keyboardshortcutsdialog.h keyboardshortcutsdialog_p.h qtkeysequenceedit.h
SOURCES += actioncollection.cpp actionmanager.cpp main.cpp mainwindow.cpp keyboardshortcutsdialog.cpp qtkeysequenceedit.cpp
