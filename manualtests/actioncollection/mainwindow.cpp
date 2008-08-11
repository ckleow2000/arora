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

#include "mainwindow.h"
#include "keyboardshortcutsdialog.h"

SubWebView::SubWebView(QWidget *parent)
    : QWebView(parent)
{
    QActionList editMenu;
    QAction *action = page()->action(QWebPage::Undo);
    action->setObjectName("edit_undo");
    editMenu.append(action);
    setMenu("Edit", editMenu);

    QActionList viewMenu;
    QAction *reloadAction = page()->action(QWebPage::Reload);
    reloadAction->setStatusTip("Reload the current webpage");
    reloadAction->setShortcut(tr("ctrl+r"));
    reloadAction->setObjectName("view_reload");
    viewMenu.append(reloadAction);
    setMenu("View", viewMenu);
    setUrl(QUrl("http://www.google.com/"));
}

SubTextEdit::SubTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    QActionList editMenu;
    QAction *action = new QAction("Undo", this);
    action->setObjectName("edit_undo");
    editMenu.append(action);
    setMenu("Edit", editMenu);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    actionManager = new ActionManager(this);
    tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);
    tabWidget->addTab(new SubWebView, "WebView");
    tabWidget->addTab(new SubTextEdit, "TextEdit");
    connect(tabWidget, SIGNAL(currentChanged(int)),
            this, SLOT(currentChanged(int)));

    QActionList quitMenu;
    QAction *action = new QAction("Quit", this);
    action->setObjectName("file_quit");
    quitMenu.append(action);
    setMenu("File", quitMenu);
    setMenu("Edit", QActionList());
    setMenu("View", QActionList());
    setMenu("Bookmarks", QActionList());

    QActionList toolsMenu;
    QAction *a = new QAction("Configure Shortcuts", this);
    connect(a, SIGNAL(triggered()),
            this, SLOT(configureShortcuts()));
    toolsMenu.append(a);
    setMenu("Tools", toolsMenu);

    setMenu("Window", QActionList());

    QMenu *bmenu = new QMenu("Bookmarks", this);
    bmenu->addAction("always here?");
    actionManager->setMenu(bmenu);
    setActionsAlwaysVisible(true);

    currentChanged(0);
}

void MainWindow::configureShortcuts()
{
    KeyboardShortcutsDialog dialog;
    dialog.exec();
}

void MainWindow::currentChanged(int index)
{
    if (ActionCollection *document = dynamic_cast<ActionCollection*>(tabWidget->widget(index)))
        actionManager->setDocumentActionCollection(document);
}
