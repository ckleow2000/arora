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

#include <QtGui/QtGui>

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);

    QFile inFile;
    QFile outFile;
    QXmlStreamWriter writer;
    QTextStream in(&inFile);
    QTextStream out(&outFile);

    // Either read in from stdin and output to stdout
    // or read in from a file and output to a file
    // Example: ./app foo.html -o bar.xbel
    bool setInput = false;
    bool setOutput = false;
    QStringList args = application.arguments();
    foreach (QString arg, args) {
        if (arg == QLatin1String("-o")) {
            setOutput = true;
        } else if (setOutput) {
            outFile.setFileName(arg);
            outFile.open(QIODevice::WriteOnly);
            writer.setDevice(&outFile);
        } else if (QFile::exists(arg)) {
            setInput = true;
            inFile.setFileName(arg);
            inFile.open(QIODevice::ReadOnly);
        } else {
            qWarning() << "Usage:" << argv[0]
                        << "[stdin|htmlfile]" << "[stdout|-o outFile]";
            return 1;
        }
    }

    if (!setInput) inFile.open(stdin, QIODevice::ReadOnly);
    if (!setOutput) {
        outFile.open(stdout, QIODevice::WriteOnly);
        writer.setDevice(&outFile);
    }
    if (inFile.openMode() == QIODevice::NotOpen
        || outFile.openMode() == QIODevice::NotOpen)
        return 1;

    writer.setAutoFormatting(true);
    writer.writeStartDocument();
    writer.writeDTD(QLatin1String("<!DOCTYPE xbel>"));
    writer.writeStartElement(QLatin1String("xbel"));
    writer.writeAttribute(QLatin1String("version"), QLatin1String("1.0"));

    // NETSCAPE-Bookmark-file-1 DTD

    do {
        // Folder
        QString line = in.readLine().trimmed();
        if (line.startsWith("<DT><H3 ")) {
            line = line.mid(7);
            int endOfH3Tag = line.indexOf('>');
            QString h3 = line.mid(0, endOfH3Tag);
            bool folded = h3.contains("FOLDED");
            QString title = line.mid(endOfH3Tag + 1, line.indexOf('<') - endOfH3Tag - 1);

            writer.writeStartElement(QLatin1String("folder"));
            writer.writeAttribute(QLatin1String("folded"), folded ? QLatin1String("yes") : QLatin1String("no"));
            writer.writeTextElement(QLatin1String("title"), title);
            continue;
        }
        if (line.contains("</DL>")) {
            writer.writeEndElement();
            continue;
        }

        // Bookmark
        if (line.startsWith("<DT><A HREF=")
            || line.startsWith("<DT><H3><A HREF=")) {
            int startURL = line.indexOf('"') + 1;
            int endURL = line.indexOf('"', startURL);
            if (startURL == -1 || endURL == -1)
                continue;
            QString url = line.mid(startURL, endURL - startURL);
            int startTitle = line.indexOf('>', endURL) + 1;
            int endTitle = line.indexOf('<', startTitle);
            QString title = line.mid(startTitle, endTitle - startTitle);

            writer.writeStartElement(QLatin1String("bookmark"));
            writer. writeAttribute(QLatin1String("href"), url);
            writer.writeTextElement(QLatin1String("title"), title);
            writer.writeEndElement();

        }

        // separator
        if (line.contains("<HR>")) {
            writer.writeEmptyElement(QLatin1String("separator"));
        }
    } while (!in.atEnd());
    writer.writeEndDocument();

    return 0;
}

