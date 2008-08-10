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

QActionList ActionCollection::menu(const QString &title)
{
    for (int i = 0; i < menuBarActions.count(); ++i)
        if (menuBarActions.at(i).first == title)
            return menuBarActions.at(i).second;
    return QActionList();
}

void ActionCollection::setMenu(const QString &title, QActionList actions)
{
    menuBarActions.append(Menu(title, actions));
}

QList<ActionCollection*> ActionCollection::collections()
{
    return m_collections;
}

bool ActionCollection::actionsAlwaysVisible() const
{
    return m_actionsAlwaysVisible;
}

void ActionCollection::setActionsAlwaysVisible(bool visible)
{
    m_actionsAlwaysVisible = visible;
}

