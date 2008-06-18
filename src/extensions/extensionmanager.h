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

#ifndef EXTENSIONMANAGER_H
#define EXTENSIONMANAGER_H

#include <qscriptvalue.h>
#include <qscriptengine.h>
#include <qaction.h>

class Extension : QObject
{
    Q_OBJECT

public:
    Extension(const QString &directory, QObject *parent = 0);
    ~Extension();

    QString extensionName() const;

    QList<QAction*> actions(const QString &menu);

    bool enabled;
    QString directory;

    QScriptEngine m_engine;
    QScriptValue object;
    QScriptValue global;

private slots:
    void executeAction();

private:
    void loadSettings();
    void saveSettings();

    QList<QAction*> actionFromValue(const QScriptValue &value);
};

#include <qobject.h>

class QFileSystemWatcher;
class ExtensionManager : public QObject
{
    Q_OBJECT

public:
    ExtensionManager(QObject *parent = 0);
    ~ExtensionManager();

    QList<QAction*> getActionsForMenu(const QString &menu);
    QString userAgentForUrl(const QUrl &url);

private slots:
    void directoryChanged(const QString &path);
    void loadExtension(const QString &extensionDirectory);

private:
    void init();
    void loadExtensions(const QString &directory);

    QList<Extension*> extensions;
    QFileSystemWatcher *fileSystemWatcher;
};

#endif // EXTENSIONS_H

