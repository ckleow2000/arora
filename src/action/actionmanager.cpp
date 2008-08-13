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

#include "actionmanager.h"
#include "actioncollection.h"

#include <qmainwindow.h>
#include <qmenubar.h>

#include <qdebug.h>

typedef QList<QAction*> QActionList;

ActionManager::ActionManager(QMainWindow *parent)
    : QObject(parent)
    , currentDocument(0)
    , m_menuBar(parent->menuBar())
{
}

void ActionManager::addPermanentActions(const ActionCollection *collection)
{
    makePartOfTheMainWindow(collection, true);
}

void ActionManager::setDocumentActionCollection(const ActionCollection *collection)
{
    if (currentDocument == collection)
        return;
    makePartOfTheMainWindow(currentDocument, false);
    currentDocument = collection;
    makePartOfTheMainWindow(currentDocument, true);
    updateMenuBar();
}

void ActionManager::setMenu(QMenu *menu)
{
    if (!menu)
        return;
    QString title = menu->title();
    if (menuBarMenus.contains(title))
        menuBarMenus[title]->deleteLater();
    todo[title] = menu;
}

void ActionManager::makePartOfTheMainWindow(const ActionCollection *actions, bool partOf)
{
    if (actions == 0)
        return;
    QWidget *p = (QWidget*)parent();
    QList<QMenu*>::const_iterator i = actions->menuBar().constBegin();
    while (i != actions->menuBar().constEnd()) {
        foreach (QAction *action, (*i)->actions()) {
            if (action->objectName().isEmpty() && !action->isSeparator())
                qWarning() << "Warning, action" << action->text() << "does have an object name";
            if (partOf)
                p->addAction(action);
            else
                p->removeAction(action);
        }
        ++i;
    }
}

void ActionManager::updateMenuBar()
{
    QList<ActionCollection*> collections = ActionCollection::collections();
    QStringList menuTitles;
    foreach (ActionCollection *collection, collections) {
        for (int i = 0; i < collection->menuBar().count(); ++i)
            menuTitles += collection->menuBar().at(i)->title();
    }
    foreach (QString title, menuTitles) {
        QMenu *menu = 0;
        if (todo.contains(title)) {
            menu = todo[title];
            todo.remove(title);
        }
        if (!menuBarMenus.contains(title)) {
            menu = m_menuBar->addMenu(title);
        }
        if (menu) {
            m_menuBar->addMenu(menu);
            menuBarMenus[title] = menu;
            connect(menu, SIGNAL(aboutToShow()),
                    this, SLOT(aboutToShowMenu()));
        }
    }

    foreach (QMenu *menu, menuBarMenus) {
        Q_ASSERT(menu);
        if (!menuTitles.contains(menu->title())) {
            menuBarMenus.remove(menu->title());
            menu->deleteLater();
        } else {
            menu->clear();
        }
    }
}

void ActionManager::aboutToShowMenu()
{
    QMenu *menu = qobject_cast<QMenu*>(sender());
    if (!menu || menu->actions().count() > 0)
        return;

    QList<ActionCollection*> collections = ActionCollection::collections();
    QAction *mergePoint = 0;
    foreach (ActionCollection *collection, collections) {
        bool disabled = (!collection->actionsAlwaysVisible() && collection != currentDocument);
        QMenu *actionCollectionMenu = collection->menu(menu->title());
        if (!actionCollectionMenu)
            continue;
        foreach (QAction *action, actionCollectionMenu->actions()) {
            if (disabled) {
                QAction *deadAction = new QAction(this);
                deadAction->setEnabled(false);
                deadAction->setText(action->text());
                deadAction->setShortcuts(action->shortcuts());
                deadAction->setIcon(action->icon());
                action = deadAction;
            }

            QActionList currentActions = menu->actions();
            bool replace = false;
            foreach (QAction *oldAction, currentActions) {
                if (oldAction->objectName() == action->objectName()) {
                    if (!disabled) {
                        menu->insertAction(oldAction, action);
                        menu->removeAction(oldAction);
                    }
                    replace = true;
                }
            }
            if (!replace) {
                if (mergePoint)
                qDebug() << "Adding" << action->text() << "before" << mergePoint->text() << actionCollectionMenu;
                else
                qDebug() << "Adding" << action->text() << action->objectName() << "at" << mergePoint << actionCollectionMenu;
                menu->insertAction(mergePoint, action);
            }
        }
        if (!mergePoint)
            mergePoint = ActionCollection::mergePoint(menu);
    }
}
