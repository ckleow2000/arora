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

#ifndef EXTENSIONS_H
#define EXTENSIONS_H

#include <qscriptvalue.h>
class Extension
{
public:
    Extension(const QString &directory);
    ~Extension();

    bool enabled;
    QString directory;

    QScriptValue object;
    QScriptValue global;
};

#include <qobject.h>

class QFileSystemWatcher;
class Extensions : public QObject
{
    Q_OBJECT

public:
    Extensions(QObject *parent = 0);
    void populateToolsMenu();

private slots:
    void loadExtension(const QString &extensionDirectory);

private:
    void loadExtensions(const QString &dir);

    QList<Extension> extensions;
    QFileSystemWatcher *fileSystemWatcher;
};

#endif // EXTENSIONS_H

