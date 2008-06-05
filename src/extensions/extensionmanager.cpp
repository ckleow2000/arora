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

#include "extensionmanager.h"
#include <QtGui/QtGui>
 #include <QScriptValueIterator>

#include "browserapplication.h"
#include "networkaccessmanager.h"
#include "cookiejar.h"
#include "history.h"

#include <QtScript/QtScript>
#include <qdesktopservices.h>
#include <qfilesystemwatcher.h>

QScriptValue qtscript_create_QWebSettings_class(QScriptEngine *engine);

Q_DECLARE_METATYPE(QScriptValue)

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

    BrowserApplication *ba = BrowserApplication::instance();
    engine.globalObject().setProperty("browserApplication", engine.newQObject(ba));
    engine.globalObject().setProperty("networkAccessManager", engine.newQObject(ba->networkAccessManager()));
    engine.globalObject().setProperty("historyManager", engine.newQObject(ba->historyManager()));
    engine.globalObject().setProperty("cookieJar", engine.newQObject(ba->cookieJar()));

    engine.globalObject().setProperty("QWebSettings",
        qtscript_create_QWebSettings_class(&engine),
        QScriptValue::SkipInEnumeration);

    object = engine.evaluate(contents, fileName);
    global = engine.globalObject();
    if (engine.hasUncaughtException()) {
        qWarning() << "error loading arora extension:" << engine.uncaughtException().toString() << engine.uncaughtExceptionLineNumber();
        enabled = false;
    }

    QString name = object.property("name").toString();

    qDebug() << object.property("name").toString() << object.property("version").toString() << object.property("description").toString() << object.property("actions").isObject() << object.property("actions").isFunction();
    //object.property("installActions").call(global);

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

QList<QAction*> Extension::actions(const QString &menu)
{
    return actionFromValue(object.property(menu).call());
}

QList<QAction*> Extension::actionFromValue(const QScriptValue &value)
{
    QList<QAction*> actions;

    if (!value.property("name").isValid()) {
        qDebug() << "got several actions";
        QScriptValueIterator it(value);
        while (it.hasNext()) {
            it.next();
            QScriptValue v = it.value();
            if (v.property("seperator").isValid()) {
                qDebug() << "got seperator";
                QAction *action = new QAction(this);
                action->setSeparator(true);
                actions.append(action);
                return actions;
            } else {
                actions += actionFromValue(v);
            }
        }
    } else {
        QString actionText = value.property("name").toString();
        QScriptValue actionProperty = value.property("action");

        QAction *action = new QAction(actionText, this);
        connect(action, SIGNAL(triggered()), this, SLOT(executeAction()));
        if (!actionProperty.isFunction()) {
            qDebug() << "got several child actions" << actionText;
            QMenu *menu = new QMenu(0);
            action->setMenu(menu);
            QList<QAction*> menuActions = actionFromValue(actionProperty);
            foreach (QAction* action, menuActions)
                menu->addAction(action);
        } else {
            qDebug() << "got action" << actionText;
            QVariant data;
            data.setValue(value);
            action->setData(data);
            QScriptValue checkedProperty = value.property("checked");
            if (checkedProperty.isValid()) {
                action->setCheckable(true);
                action->setChecked(checkedProperty.call().toBoolean());
            }
        }
        actions.append(action);
    }
    return actions;
}

void Extension::executeAction()
{
    if (QAction *action = qobject_cast<QAction*>(sender())) {
        QScriptValue v = action->data().value<QScriptValue>();
        qDebug() << v.isFunction() << v.toString();
        QScriptValue result = v.property("action").call(v);
        //extensions[0]->global);

        /*if (extensions[0]->engine.hasUncaughtException()) {
            qWarning() << "error running extension:" << extensions[0]->engine.uncaughtException().toString() << extensions[0]->engine.uncaughtExceptionLineNumber();
        }*/
        action->setChecked(v.property("checked").call(v).toBoolean());
    }
}

ExtensionManager::ExtensionManager(QObject *parent)
    : QObject(parent)
{
    QString installedExtensions = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
    QString extensionsDirectoryName = QLatin1String("extensions");
    loadExtensions(extensionsDirectoryName);
    loadExtensions(installedExtensions + QLatin1Char('/') + extensionsDirectoryName);
    //qRegisterMetaType<QScriptValue>("QScriptValue");
}

ExtensionManager::~ExtensionManager()
{
    qDeleteAll(extensions);
}

QList<QAction*> ExtensionManager::getActionsForMenu(const QString &menu)
{
    QList<QAction*> actions;
    for (int i = 0; i < extensions.count(); ++i)
        actions += extensions[i]->actions(menu);
    return actions;
}

QString ExtensionManager::userAgentForUrl(const QUrl &url)
{
    QScriptValueList args;
    args << url.toString();
    for (int i = 0; i < extensions.count(); ++i) {
        QScriptValue property = extensions[i]->object.property("userAgentForUrl");
        QScriptValue result = property.call(extensions[i]->object, args);
        if (result.isString()) {
            qDebug() << "userAgentForUrl" << property.isFunction() << result.toString();
            if (property.engine()->hasUncaughtException()) {
                qWarning() << "error running extension:" << property.engine()->uncaughtException().toString() << property.engine()->uncaughtExceptionLineNumber();
            }
            QString x = result.toString();
            if (!x.isEmpty())
                return x;
        }
    }
    return QString();
}


void ExtensionManager::loadExtension(const QString &extensionDirectory)
{
    qDebug() << "loadExtension" << extensionDirectory;
    Extension *extension = new Extension(extensionDirectory);
    extensions.append(extension);
}

void ExtensionManager::loadExtensions(const QString &dir)
{
    qDebug() << "loadExtensions" << dir;
    QDirIterator it(dir, QStringList("*.ext"), QDir::Dirs);
    while (it.hasNext())
        loadExtension(it.next());
}


/*

   void Script::loadSettings()
{
    QScriptValue settingsObject = m_engine->newObject();
    QSettings settings("settings/" + QFileInfo(m_fileName).baseName() + ".ini", QSettings::IniFormat);

    foreach (QString key, settings.allKeys()) {
        QVariant value = settings.value(key);
        QScriptValue propValue = m_engine->newVariant(value);

        if (value.type() == QVariant::Map) {
            QMap<QString, QVariant> map = value.toMap();
            propValue = m_engine->newObject();
            for (QMap<QString, QVariant>::ConstIterator it = map.constBegin(), end = map.constEnd();
                 it != end; ++it)
                propValue.setProperty(it.key(), m_engine->newVariant(it.value()));
        }

        settingsObject.setProperty(key, propValue);
    }

    m_engine->globalObject().setProperty("settings", settingsObject);
}

void Script::saveSettings()
{
    QScriptValue settingsObject = m_engine->globalObject().property("settings");
    QSettings settings("settings/" + QFileInfo(m_fileName).baseName() + ".ini", QSettings::IniFormat);

    QScriptValueIterator it(settingsObject);
    while (it.hasNext()) {
        it.next();
        if (it.name().isEmpty())
            continue;
        QScriptValue value = it.value();
        QVariant variant = value.toVariant();

        if (value.isObject()) {
            QMap<QString, QVariant> map;

            QScriptValueIterator it(value.toObject());
            while (it.hasNext()) {
                it.next();
                map.insert(it.name(), it.value().toVariant());
            }

            variant = map;
        }

        if (variant.isValid())
            settings.setValue(it.name(), variant);
    }
}
*/
