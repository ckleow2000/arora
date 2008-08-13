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

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <qobject.h>
#include <qhash.h>

class ActionCollection;
class QMainWindow;
class QMenuBar;
class QMenu;
class ActionManager : public QObject
{
    Q_OBJECT

public:
    ActionManager(QMainWindow *parent = 0);

    void addPermanentActions(const ActionCollection *collection);
    void setDocumentActionCollection(const ActionCollection *collection);
    void setMenu(QMenu *menu);

private slots:
    void aboutToShowMenu();

private:
    void makePartOfTheMainWindow(const ActionCollection *actions, bool partOf);
    void updateMenuBar();

    QHash<QString, QMenu*> menuBarMenus;
    QHash<QString, QMenu*> todo;
    const ActionCollection *currentDocument;
    QMenuBar *m_menuBar;
};

#endif // ACTIONMANAGER_H
