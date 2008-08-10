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

#ifndef KEYBOARDSHORTCUTSDIALOGS_P_H
#define KEYBOARDSHORTCUTSDIALOGS_P_H

#include "ui_action.h"

class QtKeySequenceEdit;
class KeyboardShortcutsAction : public QWidget, public Ui_Action
{
    Q_OBJECT

public:
    KeyboardShortcutsAction(QWidget *parent);

public slots:
    void updatePreviewWidget(const QModelIndex &idx);
    void keySequenceChanged(const QKeySequence &sequence);
    void newRow(const QKeySequence &sequence = QKeySequence());

private:

    QAction *currentAction;
    QList<QWidget*> shortcuts;
    QList<QtKeySequenceEdit*> editors;
};

#include <qcolumnview.h>

class KeyboardShortcutsColumnView : public QColumnView
{
    Q_OBJECT

public:
    KeyboardShortcutsColumnView(QWidget *parent = 0);

private:
    KeyboardShortcutsAction *actionWidget;

};

#endif // KEYBOARDSHORTCUTSDIALOGS_P_H
