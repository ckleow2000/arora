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

#include <QtTest/QtTest>
#include <QtGui/QtGui>
#include <actioncollection.h>

class tst_ActionCollection : public QObject
{
    Q_OBJECT

public slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

private slots:
    void actioncollection_data();
    void actioncollection();

    void actionsAlwaysVisible_data();
    void actionsAlwaysVisible();
    void collections_data();
    void collections();
    void defaultShortcuts();
    void menu_data();
    void menu();
    void setShortcuts();

    void saveShortcut_data();
    void saveShortcut();
    void restoreShortcut();
};

// Subclass that exposes the protected functions.
class SubActionCollection : public ActionCollection
{
public:
    QActionList makeList() {
        QActionList list;
        QAction *action1 = new QAction("Foo", 0);
        action1->setObjectName("bar");
        list.append(action1);
        return list;
    }

};

// This will be called before the first test function is executed.
// It is only called once.
void tst_ActionCollection::initTestCase()
{
}

// This will be called after the last test function is executed.
// It is only called once.
void tst_ActionCollection::cleanupTestCase()
{
}

// This will be called before each test function is executed.
void tst_ActionCollection::init()
{
    QSettings settings;
    settings.clear();
}

// This will be called after every test function.
void tst_ActionCollection::cleanup()
{
}

void tst_ActionCollection::actioncollection_data()
{
}

void tst_ActionCollection::actioncollection()
{
    QCOMPARE(ActionCollection::collections(), QList<ActionCollection*>());
    SubActionCollection collection;
    QCOMPARE(collection.actionsAlwaysVisible(), false);
    QCOMPARE(collection.defaultShortcuts(0), ActionCollection::Shortcuts());
    QCOMPARE(collection.menu(QString()), QActionList());
    collection.setActionsAlwaysVisible(true);
    collection.setActionsAlwaysVisible(false);
    collection.setMenu(QString(), QActionList());
    collection.setShortcuts(QString(), ActionCollection::Shortcuts());
}

void tst_ActionCollection::actionsAlwaysVisible_data()
{
    QTest::addColumn<bool>("actionsAlwaysVisible");
    QTest::newRow("false") << false;
    QTest::newRow("true") << true;
}

// public bool actionsAlwaysVisible() const
void tst_ActionCollection::actionsAlwaysVisible()
{
    QFETCH(bool, actionsAlwaysVisible);

    SubActionCollection collection;

    collection.setActionsAlwaysVisible(actionsAlwaysVisible);
    QCOMPARE(collection.actionsAlwaysVisible(), actionsAlwaysVisible);
}

void tst_ActionCollection::collections_data()
{
    QTest::addColumn<int>("count");
    QTest::newRow("0") << 0;
    QTest::newRow("1") << 1;
    QTest::newRow("5") << 5;
    QTest::newRow("10") << 10;
}

// public static QList<ActionCollection*> collections()
void tst_ActionCollection::collections()
{
    QList<ActionCollection*> collections;
    QFETCH(int, count);
    for (int i = 0; i < count; ++i)
        collections.append(new ActionCollection);

    QCOMPARE(ActionCollection::collections().count(), count);
    QCOMPARE(ActionCollection::collections(), collections);
    qDeleteAll(collections);
}

// public ActionCollection::Shortcuts defaultShortcuts(QAction const* action) const
void tst_ActionCollection::defaultShortcuts()
{
    QAction *action = new QAction("Foo", this);
    action->setObjectName("foo");
    ActionCollection::Shortcuts defaultShortcuts;
    defaultShortcuts.append(QKeySequence("Ctrl+R"));
    action->setShortcuts(defaultShortcuts);

    SubActionCollection collection;

    QActionList actions;
    actions.append(action);
    collection.setMenu("File", actions);

    QCOMPARE(collection.defaultShortcuts(action), defaultShortcuts);

    // alt
    ActionCollection::Shortcuts otherShortcuts;
    otherShortcuts.append(QKeySequence("Ctrl+Z"));
    collection.setShortcuts(action->objectName(), otherShortcuts);

    QCOMPARE(collection.defaultShortcuts(action), defaultShortcuts);
}

Q_DECLARE_METATYPE(QActionList)
void tst_ActionCollection::menu_data()
{
    QTest::addColumn<QString>("menuTitle");
    QTest::addColumn<QActionList>("menu");
    QTest::addColumn<int>("extraMenus");
    QTest::newRow("null") << QString("foo") << QActionList() << 0;
    QTest::newRow("bar") << QString("foo") << (QActionList() << 0 << 0) << 0;
    QTest::newRow("bar") << QString("foo") << (QActionList() << 0 << 0) << 1;
    QTest::newRow("bar") << QString("foo") << (QActionList() << 0 << 0) << 5;
}

// public QActionList menu(QString const& menuTitle) const
void tst_ActionCollection::menu()
{
    QFETCH(QString, menuTitle);
    QFETCH(QActionList, menu);
    QFETCH(int, extraMenus);

    SubActionCollection collection;

    for (int i = 0; i < extraMenus; ++i) {
        QActionList list;
        list << 0 << 0;
        collection.setMenu(QString("bar%1").arg(i), list);
    }

    collection.setMenu(menuTitle, menu);

    QCOMPARE(collection.menu(menuTitle), menu);
}

// public static void setShortcuts(QString const& name, ActionCollection::Shortcuts const& shortcuts)
void tst_ActionCollection::setShortcuts()
{
    SubActionCollection collection;

    // top menu
    QActionList list1 = collection.makeList();
    collection.setMenu("Test", list1);

    // another top menu
    QActionList list2;
    QAction *action2 = new QAction("Foo", this);
    action2->setObjectName("bar");
    list2.append(action2);

    // a action in a menu in a action
    QMenu *subMenu = new QMenu("sub");
    action2->setMenu(subMenu);
    QAction *action3 = new QAction("Foo", this);
    action3->setObjectName("bar");
    subMenu->addAction(action3);

    collection.setMenu("Test", list2);

    // a action in a menu in another collection
    SubActionCollection collection2;
    QActionList list1_2 = collection2.makeList();
    collection2.setMenu("Test", list1_2);

    ActionCollection::Shortcuts shortcuts;
    shortcuts.append(QKeySequence("Ctrl+Z"));
    collection.setShortcuts("bar", shortcuts);

    QCOMPARE(list1[0]->shortcuts(), shortcuts);
    QCOMPARE(action2->shortcuts(), shortcuts);
    QCOMPARE(action3->shortcuts(), shortcuts);
    QCOMPARE(list1_2[0]->shortcuts(), shortcuts);
}

Q_DECLARE_METATYPE(ActionCollection::Shortcuts)
void tst_ActionCollection::saveShortcut_data()
{
    QTest::addColumn<QString>("objectName");
    QTest::addColumn<ActionCollection::Shortcuts>("shortcuts");
    QTest::addColumn<QStringList>("settingsValue");
    QTest::addColumn<QStringList>("keys");
    QTest::addColumn<bool>("reset");
    QKeySequence sequence = QKeySequence("Ctrl+Z");
    QStringList out;
    out << sequence.toString();
    QStringList keys;
    keys.append("bar");
    QTest::newRow("no name, seq") << QString() << (ActionCollection::Shortcuts() << sequence) << QStringList() << QStringList() << false;
    QTest::newRow("name, seq") << QString("bar") << (ActionCollection::Shortcuts() << sequence) << out << keys << false;
    QTest::newRow("name, no seq") << QString("bar") << (ActionCollection::Shortcuts()) << QStringList() << QStringList() << false;
    QTest::newRow("name, seq, res") << QString("bar") << (ActionCollection::Shortcuts() << sequence) << QStringList() << QStringList() << true;
}

void tst_ActionCollection::saveShortcut()
{
    QFETCH(QString, objectName);
    QFETCH(ActionCollection::Shortcuts, shortcuts);
    QFETCH(QStringList, settingsValue);
    QFETCH(QStringList, keys);
    QFETCH(bool, reset);
    SubActionCollection collection;
    QActionList list1 = collection.makeList();
    list1[0]->setObjectName(objectName);
    ActionCollection::Shortcuts originalShortcuts = list1[0]->shortcuts();
    collection.setMenu("Test", list1);

    collection.setShortcuts("bar", shortcuts);

    if (reset)
        collection.setShortcuts("bar", originalShortcuts);

    QSettings settings;
    settings.beginGroup("shortcuts");
    QCOMPARE(settings.allKeys(), keys);

    QCOMPARE(settings.value("bar").toStringList(), settingsValue);
}

void tst_ActionCollection::restoreShortcut()
{
    QSettings settings;
    settings.beginGroup("shortcuts");
    QKeySequence sequence = QKeySequence("Ctrl+Z");
    ActionCollection::Shortcuts shortcuts;
    shortcuts.append(sequence);
    settings.setValue("bar", QStringList() << sequence.toString());

    SubActionCollection collection;
    QActionList list;
    QAction *action = new QAction("Foo", 0);
    action->setObjectName("bar");
    list.append(action);
    collection.setMenu("Test", list);
    QCOMPARE(action->shortcuts(), shortcuts);
}

QTEST_MAIN(tst_ActionCollection)
#include "tst_actioncollection.moc"

