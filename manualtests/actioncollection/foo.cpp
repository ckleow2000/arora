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

#include "actioncollection.h"
#include "keyboardshortcutsdialog.h"

ActionCollection::ActionCollection(QObject *parent)
    : QObject(parent)
    , dialog(0)
{
}

void ActionCollection::addAction(QAction *action)
{
    actions.append(action);
}

void ActionCollection::configure(QMenu *menu, QWidget *parent)
{
    if (!dialog)
        dialog = new KeyboardShortcutsDialog(parent);
    dialog->update(menu, this);
    dialog->show();
}

void ActionCollection::save()
{
}

void ActionCollection::load()
{
}

XbelWriter::XbelWriter()
{
    setAutoFormatting(true);
}

bool XbelWriter::write(const QString &fileName, const BookmarkNode *root)
{
    QFile file(fileName);
    if (!root || !file.open(QFile::WriteOnly))
        return false;
    return write(&file, root);
}

bool XbelWriter::write(QIODevice *device, const BookmarkNode *root)
{
    setDevice(device);

    writeStartDocument();
    writeDTD(QLatin1String("<!DOCTYPE xbel>"));
    writeStartElement(QLatin1String("xbel"));
    writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
    if (root->type() == BookmarkNode::Root) {
        for (int i = 0; i < root->children().count(); ++i)
            writeItem(root->children().at(i));
    } else {
        writeItem(root);
    }

    writeEndDocument();
    return true;
}

void XbelWriter::writeItem(const BookmarkNode *parent)
{
    switch (parent->type()) {
    case BookmarkNode::Folder:
        writeStartElement(QLatin1String("folder"));
        writeAttribute(QLatin1String("folded"), parent->expanded ? QLatin1String("no") : QLatin1String("yes"));
        writeTextElement(QLatin1String("title"), parent->title);
        for (int i = 0; i < parent->children().count(); ++i)
            writeItem(parent->children().at(i));
        writeEndElement();
        break;
    case BookmarkNode::Bookmark:
        writeStartElement(QLatin1String("bookmark"));
        if (!parent->url.isEmpty())
            writeAttribute(QLatin1String("href"), parent->url);
        writeTextElement(QLatin1String("title"), parent->title);
        if (!parent->desc.isEmpty())
            writeAttribute(QLatin1String("desc"), parent->desc);
        writeEndElement();
        break;
    case BookmarkNode::Separator:
        writeEmptyElement(QLatin1String("separator"));
        break;
    default:
        break;
    }
}


