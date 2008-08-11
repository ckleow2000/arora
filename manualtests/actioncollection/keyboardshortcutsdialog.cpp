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

#include "keyboardshortcutsdialog.h"
#include "keyboardshortcutsdialog_p.h"

#include "actioncollection.h"
#include "qtkeysequenceedit.h"

#include <qmenu.h>
#include <qstandarditemmodel.h>

#include <qdebug.h>

KeyboardShortcutsAction::KeyboardShortcutsAction(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
    newRow();
    connect(defaultButton, SIGNAL(clicked()),
            this, SLOT(resetToDefault()));
}

void KeyboardShortcutsAction::updatePreviewWidget(const QModelIndex &idx)
{
    name->setText(idx.data().toString());

    QVariant v = idx.data(KeyboardShortcutsDialog::ActionRole);
    currentAction = (QAction *) v.value<void *>();

    v = idx.data(KeyboardShortcutsDialog::CollectionRole);
    collection = (ActionCollection *) v.value<void *>();

    if (!currentAction)
        return;

    icon->setText(QString());
    icon->setPixmap(currentAction->icon().pixmap(32, 32));
    QString infoString = currentAction->statusTip();
    if (infoString.isEmpty())
        infoString = currentAction->toolTip();
    if (infoString.isEmpty())
        infoString = currentAction->whatsThis();
    if (infoString == currentAction->text())
        infoString = QString();
    info->setText(infoString);

    QList<QKeySequence> actions = currentAction->shortcuts();
    if (!actions.isEmpty())
        editor->setKeySequence(actions.first());
}

void KeyboardShortcutsAction::newRow()
{
    editor = new QtKeySequenceEdit(this);
    connect(editor, SIGNAL(keySequenceChanged(const QKeySequence &)),
            this, SLOT(keySequenceChanged(const QKeySequence &)));
    verticalLayout->addWidget(editor);
}

void KeyboardShortcutsAction::resetToDefault()
{
    editor->setKeySequence(collection->defaultShortcuts(currentAction).value(0));
}

void KeyboardShortcutsAction::keySequenceChanged(const QKeySequence &sequence)
{
    ActionCollection::Shortcuts shortcuts;
    shortcuts.append(sequence);
    collection->setShortcuts(currentAction->objectName(), shortcuts);
}

KeyboardShortcutsColumnView::KeyboardShortcutsColumnView(QWidget *parent)
    : QColumnView(parent)
{
    actionWidget = new KeyboardShortcutsAction(this);
    actionWidget->resize(actionWidget->sizeHint());
    actionWidget->show();
    setPreviewWidget(actionWidget);
    connect(this, SIGNAL(updatePreviewWidget(const QModelIndex &)),
            actionWidget, SLOT(updatePreviewWidget(const QModelIndex &)));

    QList<int> widths;
    widths.append(50);
    widths.append(80);
    setColumnWidths(widths);
}

KeyboardShortcutsDialog::KeyboardShortcutsDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , m_model(new QStandardItemModel(this))
{
    setupUi(this);
    connect(search, SIGNAL(textChanged(const QString &)),
            this, SLOT(populateModel()));
    populateModel();
}

QModelIndex KeyboardShortcutsDialog::insertRow(const QString &title, const QModelIndex &parent)
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
    }

    return m_model->index(row, 0, parent);
}

void KeyboardShortcutsDialog::addAction(ActionCollection *collection, const QModelIndex &parent, QAction *action)
{
    QString title = action->text();
    QModelIndex idx = insertRow(title, parent);

    QVariant v = qVariantFromValue((void *) action);
    m_model->setData(idx, v, ActionRole);
    v = qVariantFromValue((void *) collection);
    m_model->setData(idx, v, CollectionRole);

    qDebug() << title << action->objectName() << action->shortcuts();
    if (action->menu()) {
        foreach (QAction *a, action->menu()->actions())
            addAction(collection, idx, a);
    }
}

void KeyboardShortcutsDialog::addCollection(ActionCollection *collection)
{
    QModelIndex parent;
    for (int i = 0; i < collection->menuBarActions.count(); ++i) {
        QString title = collection->menuBarActions.at(i).first;
        QModelIndex idx = insertRow(title, QModelIndex());
        QActionList list = collection->menuBarActions.at(i).second;
        for (int i = 0; i < list.count(); ++i)
            addAction(collection, idx, list.at(i));
    }
}

void KeyboardShortcutsDialog::populateModel()
{
    columnView->setUpdatesEnabled(false);
    columnView->setModel(0);
    m_model->clear();
    QList<ActionCollection*> collections = ActionCollection::collections();
    QStringList menuTitles;
    foreach (ActionCollection *collection, collections)
        addCollection(collection);

    columnView->setModel(m_model);
    columnView->previewWidget()->setVisible(m_model->rowCount() > 0);

    columnView->setUpdatesEnabled(true);
}

