/*
 * Copyright 2008 Benjamin C. Meyer <ben@meyerhome.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "actioncollection.h"

#include <qmenu.h>
#include <qsettings.h>

#include <qdebug.h>

// static list of collections
QList<ActionCollection*>ActionCollection::m_collections;

ActionCollection::ActionCollection()
    : m_actionsAlwaysVisible(false)
{
    m_collections.append(this);
}

ActionCollection::~ActionCollection()
{
    m_collections.removeOne(this);
}

/*!
    Returns the list of actions for the menu \a title
 */
QMenu *ActionCollection::menu(const QString &menuTitle) const
{
    for (int i = 0; i < m_menuBar.count(); ++i)
        if (m_menuBar.at(i)->title() == menuTitle)
            return m_menuBar[i];
    return 0;
}

/*!
    Sets actions on the menu menuTitle and loads any shortcuts from the settings file.
 */
void ActionCollection::addMenu(QMenu *menu)
{
    if (!menu)
        return;
    m_menuBar.append(menu);

    QSettings settings;
    settings.beginGroup(QLatin1String("shortcuts"));
    QStringList keys = settings.allKeys();
    if (keys.isEmpty())
        return;
    foreach (QAction *action, menu->actions())
        readActionShortcutFromSettings(action, keys);
}

void ActionCollection::readActionShortcutFromSettings(QAction *action, const QStringList &keys)
{
    if (!action)
        return;
    QString name = action->objectName();
    if (keys.contains(name)) {
        QSettings settings;
        settings.beginGroup(QLatin1String("shortcuts"));
        if (!name.isEmpty() && settings.contains(name)) {
            QStringList list = settings.value(name).toStringList();
            Shortcuts shortcuts;
            foreach (QString string, list)
                shortcuts.append(QKeySequence::fromString(string));
            setShortcuts(name, shortcuts);
        }
    }

    if (action->menu())
        foreach (QAction *child, action->menu()->actions())
            readActionShortcutFromSettings(child, keys);
}

/*
    Return a list of all of the current ActionCollection's
 */
QList<ActionCollection*> ActionCollection::collections()
{
    return m_collections;
}

/*
    Returns true if these actions should always be shown in the menu or not.

    Actions that are not always visible are only shown by ActionManager if
    they are the current document.
 */
bool ActionCollection::actionsAlwaysVisible() const
{
    return m_actionsAlwaysVisible;
}

void ActionCollection::setActionsAlwaysVisible(bool visible)
{
    m_actionsAlwaysVisible = visible;
}

/*
    Returns the default shortcuts for action
 */
ActionCollection::Shortcuts ActionCollection::defaultShortcuts(const QAction *action) const
{
    if (!action)
        return Shortcuts();
    QString name = action->objectName();
    if (m_defaultShortcuts.contains(name))
        return m_defaultShortcuts[name];
    return action->shortcuts();
}

/*
    Sets all actions with the objectName \a name to have the shortcuts \a shortcuts
 */
void ActionCollection::setShortcuts(const QString &name, const Shortcuts &shortcuts)
{
    foreach (ActionCollection *collection, m_collections) {
        foreach (QMenu *menu, collection->m_menuBar) {
            foreach (QAction *action, menu->actions()) {
                collection->setShortcuts(action, name, shortcuts);
            }
        }
    }
}

/*
    \internal

    Check action and all of its children to see if it matches name and if so
    set its shortcuts to \a shortcuts.  If a match is found the new value is
    saved to disk if it is not the default, but if it is then it is removed from the disk.
 */
void ActionCollection::setShortcuts(QAction *action, const QString &name, const Shortcuts &shortcuts)
{
    if (!action || name.isEmpty())
        return;

    if (action->objectName() == name) {
        if (!m_defaultShortcuts.contains(name))
            m_defaultShortcuts[name] = action->shortcuts();
        action->setShortcuts(shortcuts);

        QSettings settings;
        settings.beginGroup(QLatin1String("shortcuts"));
        if (shortcuts != defaultShortcuts(action)) {
            QStringList list;
            foreach (QKeySequence sequence, shortcuts)
                list.append(sequence.toString());
            settings.setValue(name, list);
        } else {
            settings.remove(name);
        }
    }

    if (action->menu())
        foreach (QAction *child, action->menu()->actions())
            setShortcuts(child, name, shortcuts);
}

