/* AtCore KDE Libary for 3D Printers
    Copyright (C) <2016>

    Authors:
        Tomaz Canabrava <tcanabrava@kde.org>
        Chris Rizzitello <rizzitello@kde.org>
        Patrick José Pereira <patrickjp@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <QLoggingCategory>
#include <QString>

#include "atcore.h"
#include "repetierplugin.h"

Q_LOGGING_CATEGORY(REPETIER_PLUGIN, "org.kde.atelier.core.firmware.repetier")

QString RepetierPlugin::name() const
{
    return QStringLiteral("Repetier");
}

bool RepetierPlugin::isSdSupported() const
{
    return true;
}

RepetierPlugin::RepetierPlugin()
{
    qCDebug(REPETIER_PLUGIN) << name() << " plugin loaded!";
}

void RepetierPlugin::validateCommand(const QString &lastMessage)
{
    if (lastMessage.contains(QStringLiteral("End file list"))) {
        core()->setReadingSdCardList(false);
    } else if (core()->isReadingSdCardList()) {
        // Below is to not add directories
        if (!lastMessage.endsWith(QChar::fromLatin1('/'))) {
            QString fileName = lastMessage;
            fileName.chop(fileName.length() - fileName.lastIndexOf(QChar::fromLatin1(' ')));
            core()->appendSdCardFileList(fileName);
        }
    } else {
        if (lastMessage.contains(QStringLiteral("SD card"))) {
            if (lastMessage.contains(QStringLiteral("inserted"))) {
                core()->setSdMounted(true);
            } else if (lastMessage.contains(QStringLiteral("removed"))) {
                core()->setSdMounted(false);
            }
        } else if (lastMessage.contains(QStringLiteral("Begin file list"))) {
            core()->setSdMounted(true);
            core()->setReadingSdCardList(true);
            core()->clearSdCardFileList();
        } else if (lastMessage.contains(QStringLiteral("SD printing byte"))) {
            if (lastMessage.contains(QStringLiteral("SD printing byte 0/0"))) {
                // not printing a file
                return;
            }
            if (core()->state() != AtCore::BUSY) {
                // This should only happen if Attached to an Sd printing machine.
                // Just tell the client were starting a job like normal.
                // For this to work the client should check if sdCardPrintStatus()
                // Upon the Connection to a known firmware with sdSupport
                core()->setState(AtCore::STARTPRINT);
                core()->setState(AtCore::BUSY);
            }
            QString temp = lastMessage;
            temp.replace(QStringLiteral("SD printing byte"), QString());
            qlonglong total = temp.mid(temp.lastIndexOf(QChar::fromLatin1('/')) + 1, temp.length() - temp.lastIndexOf(QChar::fromLatin1('/'))).toLongLong();
            if (total) {
                temp.chop(temp.length() - temp.lastIndexOf(QChar::fromLatin1('/')));
                qlonglong remaining = total - temp.toLongLong();
                float progress = float(total - remaining) * 100 / float(total);
                emit core()->printProgressChanged(progress);
                if (progress >= 100) {
                    core()->setState(AtCore::FINISHEDPRINT);
                    core()->setState(AtCore::IDLE);
                }
            } else {
                core()->setState(AtCore::FINISHEDPRINT);
                core()->setState(AtCore::IDLE);
            }
        }
        if (lastMessage.contains(QStringLiteral("ok"))) {
            emit readyForCommand();
        }
    }
}
