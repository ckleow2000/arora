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

#ifndef KEYBOARDSHORTCUTSDIALOGS_H
#define KEYBOARDSHORTCUTSDIALOGS_H

#include <qdialog.h>
#include "ui_keyboardshortcutsdialog.h"

class ActionCollection;
class QMenu;
class QStandardItem;
class QStandardItemModel;

class KeyboardShortcutsDialog : public QDialog, public Ui_ShortcutDialog
{
    Q_OBJECT

public:
    enum Roles {
        ActionRole = Qt::UserRole + 1,
        CollectionRole = Qt::UserRole + 2,
    };

    KeyboardShortcutsDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);

private slots:
    void populateModel();

private:
    QModelIndex insertRow(const QString &title, const QModelIndex &parent);
    void addAction(ActionCollection *collection, const QModelIndex &parent, QAction *action);
    void addCollection(ActionCollection *collection);

    QStandardItemModel *m_model;
};

#endif // KEYBOARDSHORTCUTSDIALOGS_H

