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

#include "keyboardshortcutsdialog_p.h"
#include "keyboardshortcutsdialog.h"
#include "qtkeysequenceedit.h"

#include <qmenu.h>
#include <qstandarditemmodel.h>
#include "actioncollection.h"

#include <qdebug.h>

KeyboardShortcutsAction::KeyboardShortcutsAction(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    addButton->setIcon(QIcon(":addtab.png"));
    connect(addButton, SIGNAL(clicked()),
            this, SLOT(newRow()));

}

KeyboardShortcutsColumnView::KeyboardShortcutsColumnView(QWidget *parent)
    : QColumnView(parent)
{
    actionWidget = new KeyboardShortcutsAction(this);
    actionWidget->resize(actionWidget->sizeHint());
    actionWidget->addButton->setAutoRaise(true);
    actionWidget->show();
    setPreviewWidget(actionWidget);
    connect(this, SIGNAL(updatePreviewWidget(const QModelIndex &)),
            actionWidget, SLOT(updatePreviewWidget(const QModelIndex &)));

    QList<int> widths;
    widths.append(50);
    widths.append(50);
    setColumnWidths(widths);
}

void KeyboardShortcutsAction::updatePreviewWidget(const QModelIndex &idx)
{
    tooltip->setText(idx.data().toString());
    QVariant v = idx.data(KeyboardShortcutsDialog::ActionRole);
    currentAction = (QAction *) v.value<void *>();
    if (!currentAction)
        return;

    foreach (QWidget *w, shortcuts)
        w->deleteLater();
    editors.clear();
    shortcuts.clear();
    QList<QKeySequence> actions = currentAction->shortcuts();
    for (int i = 0; i < actions.count(); ++i) {
        newRow(actions[i]);
    }
}

void KeyboardShortcutsAction::newRow(const QKeySequence &sequence)
{
    QWidget *widget = new QWidget(this);
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setContentsMargins(0, 0, 0, 0);
    QtKeySequenceEdit *shortcut = new QtKeySequenceEdit(widget);
    shortcut->setKeySequence(sequence);
    editors.append(shortcut);
    connect(shortcut, SIGNAL(keySequenceChanged(const QKeySequence &)),
            this, SLOT(keySequenceChanged(const QKeySequence &)));

    gridLayout->addWidget(shortcut, 0, 0, 1, 1);

    QToolButton *toolButton = new QToolButton(widget);
    toolButton->setObjectName(QString::fromUtf8("toolButton"));
    toolButton->setIcon(QIcon(":closetab.png"));
    gridLayout->addWidget(toolButton, 0, 1, 1, 1);
    widget->setLayout(gridLayout);
    verticalLayout->addWidget(widget);
    shortcuts.append(widget);
    resize(width(), sizeHint().height());
}


void KeyboardShortcutsAction::keySequenceChanged(const QKeySequence &)
{
    QList<QKeySequence> actions;
    foreach (QtKeySequenceEdit *editor, editors)
        actions.append(editor->keySequence());
    currentAction->setShortcuts(actions);
}


KeyboardShortcutsDialog::KeyboardShortcutsDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    setupUi(this);
    m_model = new QStandardItemModel(this);
    connect(search, SIGNAL(textChanged(const QString &)),
            this, SLOT(generate()));
    generate();
    connect(m_model, SIGNAL(itemChanged(QStandardItem *)),
            this, SLOT(itemChanged(QStandardItem *)));
}

void KeyboardShortcutsDialog::itemChanged(QStandardItem *item)
{
    qDebug() << item->text() << "changed";
}

QModelIndex KeyboardShortcutsDialog::insert(const QString &title, const QModelIndex &parent)
{
    QString searchString = search->text().toLower();
    int row = -1;
    for (int i = 0; i < m_model->rowCount(parent); ++i) {
        if (m_model->index(i, 0, parent).data().toString() == title) {
            row = i;
            break;
        }
    }
    bool add = searchString.isEmpty() || (!searchString.isEmpty() && title.toLower().contains(searchString));
    if (row == -1 && add) {
        if (m_model->columnCount(parent) == 0)
            m_model->insertColumn(0, parent);
        m_model->insertRow(m_model->rowCount(parent), parent);
        row = m_model->rowCount(parent) - 1;
        QModelIndex idx = m_model->index(row, 0, parent);
        m_model->setData(idx, title);
        qDebug() << "add" << title;
    }

    return m_model->index(row, 0, parent);
}

void KeyboardShortcutsDialog::addAction(const QModelIndex &parent, QAction *action)
{
    QString title = action->text();
    QModelIndex idx = insert(title, parent);

    QVariant v = qVariantFromValue((void *) action);
    m_model->setData(idx, v, ActionRole);

    if (action->menu()) {
        foreach (QAction *a, action->menu()->actions())
            addAction(idx, a);
    }
}

void KeyboardShortcutsDialog::addCollection(ActionCollection *collection)
{
    QModelIndex parent;
    for (int i = 0; i < collection->menuBarActions.count(); ++i) {
        QString title = collection->menuBarActions.at(i).first;
        QModelIndex idx = insert(title, QModelIndex());
        QActionList list = collection->menuBarActions.at(i).second;
        for (int i = 0; i < list.count(); ++i)
            addAction(idx, list.at(i));
    }
}

void KeyboardShortcutsDialog::generate()
{
    columnView->setModel(0);
    m_model->clear();
    m_model->insertColumn(0);
    QList<ActionCollection*> collections = ActionCollection::collections();
    QStringList menuTitles;
    foreach (ActionCollection *collection, collections)
        addCollection(collection);

    columnView->setModel(m_model);
}

