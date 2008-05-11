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

#include "extensions.h"

#include "browserapplication.h"
#include "networkaccessmanager.h"
#include "cookiejar.h"
#include "history.h"

#include <QtScript/QtScript>
#include <qdesktopservices.h>
#include <qfilesystemwatcher.h>

Extension::Extension(const QString &directory)
    : enabled(true)
    , directory(directory)
{
    QString fileName = directory + "/extension.js";
    QFile scriptFile(fileName);
    if (!scriptFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open extention" << fileName << scriptFile.errorString();
        return;
    }

    QTextStream stream(&scriptFile);
    QString contents = stream.readAll();
    scriptFile.close();
    QScriptEngine engine;

    BrowserApplication *ba = BrowserApplication::instance();
    engine.globalObject().setProperty("browserApplication", engine.newQObject(ba));
    engine.globalObject().setProperty("networkAccessManager", engine.newQObject(ba->networkAccessManager()));
    engine.globalObject().setProperty("historyManager", engine.newQObject(ba->historyManager()));
    engine.globalObject().setProperty("cookieJar", engine.newQObject(ba->cookieJar()));
//    engine.globalObject().setProperty("extentionLocation", engine.newObject(directory));

    object = engine.evaluate(contents, fileName);
    global = engine.globalObject();
    if (engine.hasUncaughtException()) {
        qWarning() << "error loading extension:" << engine.uncaughtException().toString() << engine.uncaughtExceptionLineNumber();
        enabled = false;
    }

    QString name = object.property("name").toString();

    qDebug() << object.property("name").toString() << object.property("version").toString() << object.property("description").toString();
    object.property("installActions").call(global);

    QSettings settings;
    settings.beginGroup(QLatin1String("Extensions"));
    settings.beginGroup(name);
    enabled = settings.value(QLatin1String("Enabled"), enabled).toBool();
    settings.endGroup();
    settings.endGroup();
}

Extension::~Extension()
{
    // save if enabled or not
}

Extensions::Extensions(QObject *parent)
    : QObject(parent)
{
    QString installedExtensions = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    QString extensionsDirectoryName = QLatin1String("extensions");
    loadExtensions(extensionsDirectoryName);
    loadExtensions(installedExtensions + QLatin1Char('/') + extensionsDirectoryName);
}

void Extensions::populateToolsMenu()
{
    for (int i = 0; i < extensions.count(); ++i) {
        extensions[i].object.property("menuAction").call(extensions[i].global);
    }
}

void Extensions::loadExtension(const QString &extensionDirectory)
{
    qDebug() << "loadExtension" << extensionDirectory;
    Extension extension(extensionDirectory);
    extensions.append(extension);
}

void Extensions::loadExtensions(const QString &dir)
{
    qDebug() << "loadExtensions" << dir;
    QDirIterator it(dir, QStringList("*.ext"), QDir::Dirs);
    while (it.hasNext())
        loadExtension(it.next());
}
