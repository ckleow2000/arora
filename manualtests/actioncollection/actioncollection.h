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

#ifndef ACTIONCOLLECTION_H
#define ACTIONCOLLECTION_H

#include <qaction.h>

typedef QList<QAction*> QActionList;

/*
    ActionCollection is a container for a collection of actions.  ActionCollection
    is designed to be used in conjunction with ActionManager and KeyboardShortcutsDialog.

    Actions are stored in the same layout that they should appear in the menuBar.

    For example the list of actions (including sub menu's) for the file menu should
    be placed in a QActionList and then set on the action collection:
    actionCollection.setMenu("File", actions);

    ActionCollection takes care of loading and saving shortcuts values from settings if they
    are not the default values.
 */
class ActionCollection
{

public:
    typedef QPair<QString, QActionList> Menu;
    typedef QList<QKeySequence> Shortcuts;

    ActionCollection();
    ~ActionCollection();

    QActionList menu(const QString &menuTitle) const;
    void setMenu(const QString &menuTitle, const QActionList &actions);

    bool actionsAlwaysVisible() const;
    void setActionsAlwaysVisible(bool visible);

    Shortcuts defaultShortcuts(const QAction *action) const;
    static void setShortcuts(const QString &name, const Shortcuts &shortcuts);

    static QList<ActionCollection*> collections();

    QList<Menu> menuBarActions;

private:
    void readActionShortcutFromSettings(QAction *action, const QStringList &keys);
    void setShortcuts(QAction *action, const QString &name, const Shortcuts &shortcuts);

    QHash<QString, Shortcuts> m_defaultShortcuts;
    static QList<ActionCollection*> m_collections;
    bool m_actionsAlwaysVisible;
};

#endif // ACTIONCOLLECTION_H

