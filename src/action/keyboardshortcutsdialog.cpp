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
#include <qscrollbar.h>
#include <qtimer.h>

#include <qdebug.h>

KeyboardShortcutsAction::KeyboardShortcutsAction(QWidget *parent)
    : QWidget(parent)
    , m_model(0)
{
    setupUi(this);
    connect(editor, SIGNAL(keySequenceChanged(const QKeySequence &)),
            this, SLOT(keySequenceChanged(const QKeySequence &)));
    connect(defaultButton, SIGNAL(clicked()),
            this, SLOT(resetToDefault()));
}

void KeyboardShortcutsAction::updatePreviewWidget(const QModelIndex &idx)
{
    conflict->hide();
    name->setText(idx.data().toString());

    QVariant v = idx.data(KeyboardShortcutsDialog::ActionRole);
    m_action = (QAction *) v.value<void *>();

    v = idx.data(KeyboardShortcutsDialog::CollectionRole);
    m_collection = (ActionCollection *) v.value<void *>();

    if (!m_action)
        return;

    icon->setText(QString());
    QIcon actionIcon = m_action->icon();
    icon->setVisible(!actionIcon.isNull());
    icon->setPixmap(m_action->icon().pixmap(32, 32));
    QString infoString = m_action->statusTip();
    if (infoString.isEmpty())
        infoString = m_action->toolTip();
    if (infoString.isEmpty())
        infoString = m_action->whatsThis();
    if (infoString == m_action->text())
        infoString = QString();
    info->setText(infoString);

    QList<QKeySequence> currentShortcuts = m_action->shortcuts();
    if (!currentShortcuts.isEmpty())
        editor->setKeySequence(currentShortcuts.first());
    defaultButton->setEnabled(currentShortcuts != m_collection->defaultShortcuts(m_action));
    findConflict(currentShortcuts);
}

void KeyboardShortcutsAction::resetToDefault()
{
    ActionCollection::Shortcuts shortcuts = m_collection->defaultShortcuts(m_action);
    editor->setKeySequence(shortcuts.value(0));
    m_collection->setShortcuts(m_action->objectName(), shortcuts);
    defaultButton->setEnabled(false);
    findConflict(shortcuts);
}

void KeyboardShortcutsAction::keySequenceChanged(const QKeySequence &sequence)
{
    ActionCollection::Shortcuts shortcuts;
    shortcuts.append(sequence);
    m_collection->setShortcuts(m_action->objectName(), shortcuts);

    QList<QKeySequence> currentShortcuts = m_action->shortcuts();
    defaultButton->setEnabled(currentShortcuts != m_collection->defaultShortcuts(m_action));

    findConflict(currentShortcuts);
}

void KeyboardShortcutsAction::findConflict(const ActionCollection::Shortcuts &shortcuts)
{
    if (shortcuts.isEmpty()) {
        conflict->setVisible(false);
        return;
    }
    QAction *isConflict = findConflict(QModelIndex(), shortcuts);
    conflict->setVisible(isConflict);
    if (isConflict)
        conflict->setText(tr("Conflict with: %1").arg(isConflict->text()));
}

QAction *KeyboardShortcutsAction::findConflict(const QModelIndex &parent,
        const ActionCollection::Shortcuts &shortcuts)
{
    if (!m_model)
        return false;
    for (int i = 0; i < m_model->rowCount(parent); ++i) {
        QModelIndex idx = m_model->index(i, 0, parent);
        QVariant v = idx.data(KeyboardShortcutsDialog::ActionRole);
        QAction *action = (QAction *) v.value<void *>();
        if (action && action != m_action && action->shortcuts() == shortcuts)
            return action;
        QAction *r = findConflict(idx, shortcuts);
        if (r)
            return r;
    }
    return 0;
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
}

void KeyboardShortcutsColumnView::setModel(QAbstractItemModel *model)
{
    actionWidget->m_model = model;
    QColumnView::setModel(model);
}

KeyboardShortcutsDialog::KeyboardShortcutsDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , m_model(new QStandardItemModel(this))
{
    setupUi(this);
    connect(search, SIGNAL(textChanged(const QString &)),
            this, SLOT(populateModel()));
    populateModel();

    QFontMetrics fm = columnView->fontMetrics();
    int firstColumnWidth = 0;
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex idx = m_model->index(i, 0);
        QString title = idx.data().toString();
        firstColumnWidth = qMax(fm.width(title) + fm.height(), firstColumnWidth);
    }
    firstColumnWidth += columnView->verticalScrollBar()->sizeHint().width();
    QList<int> widths;
    widths.append(firstColumnWidth);
    widths.append(firstColumnWidth * 1.5);
    columnView->setColumnWidths(widths);
    QTimer::singleShot(1, this, SLOT(selectFirstItem()));
}

QModelIndex KeyboardShortcutsDialog::insertRow(const QString &title, const QModelIndex &parent)
{
    QString strippedTitle = title;
    strippedTitle.replace(QLatin1Char('&'), QString());
    QString searchString = search->text().toLower();
    int row = -1;
    for (int i = 0; i < m_model->rowCount(parent); ++i) {
        if (m_model->index(i, 0, parent).data().toString() == strippedTitle) {
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
        m_model->setData(idx, strippedTitle);
    }

    return m_model->index(row, 0, parent);
}

void KeyboardShortcutsDialog::addAction(ActionCollection *m_collection, const QModelIndex &parent, QAction *action)
{
    QString title = action->text();
    QModelIndex idx = insertRow(title, parent);

    QVariant v = qVariantFromValue((void *) action);
    m_model->setData(idx, v, ActionRole);
    v = qVariantFromValue((void *) m_collection);
    m_model->setData(idx, v, CollectionRole);

    qDebug() << title << action->objectName() << action->shortcuts();
    if (action->menu()) {
        foreach (QAction *a, action->menu()->actions())
            addAction(m_collection, idx, a);
    }
}

void KeyboardShortcutsDialog::addCollection(ActionCollection *m_collection)
{
    QModelIndex parent;
    for (int i = 0; i < m_collection->menuBar().count(); ++i) {
        QString title = m_collection->menuBar().at(i)->title();
        QModelIndex idx = insertRow(title, QModelIndex());
        QList<QAction*> list = m_collection->menuBar().at(i)->actions();
        for (int i = 0; i < list.count(); ++i)
            addAction(m_collection, idx, list.at(i));
    }
}

void KeyboardShortcutsDialog::populateModel()
{
    columnView->setUpdatesEnabled(false);
    columnView->setModel(0);
    m_model->clear();
    QList<ActionCollection*> collections = ActionCollection::collections();
    QStringList menuTitles;
    foreach (ActionCollection *m_collection, collections)
        addCollection(m_collection);

    columnView->setModel(m_model);
    columnView->previewWidget()->setVisible(m_model->rowCount() > 0);

    columnView->setUpdatesEnabled(true);
}

void KeyboardShortcutsDialog::selectFirstItem()
{
    columnView->selectionModel()->select(m_model->index(0, 0), QItemSelectionModel::SelectCurrent);
    columnView->setFocus();
}

