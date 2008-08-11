INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

FORMS += \
    action.ui \
    keyboardshortcutsdialog.ui

HEADERS += \
    actioncollection.h \
    actionmanager.h \
    keyboardshortcutsdialog.h \
    keyboardshortcutsdialog_p.h \
    qtkeysequenceedit.h

SOURCES += \
    actioncollection.cpp \
    actionmanager.cpp \
    keyboardshortcutsdialog.cpp \
    qtkeysequenceedit.cpp