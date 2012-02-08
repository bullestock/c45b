// Copyright 2011 Torsten Martinsen <bullestock@bullestock.net>

// This file is part of c45b.

// c45b is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// c45b is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with c45b.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <iomanip>

#include "c45butils.h"
#include "platform.h"
#include "serport.h"

using namespace std;

C45BSerialPort::C45BSerialPort(QString device,
                               bool verbose)
    : QSerialPort(device),
      m_verbose(verbose)
{
}

C45BSerialPort::~C45BSerialPort()
{
}

bool C45BSerialPort::init(int baudRate)
{
    // To set the baud rate from an integer, use QPortSettings::set(QString).
    // We must do this first, since it resets all settings first.
    TNX::QPortSettings settings;
    if (baudRate <= 0)
        baudRate = 19200;
    if (!settings.set(QString("%1").arg(baudRate)))
    {
        cout << "Cannot set baud rate to " << baudRate << endl;
        return false;
    }
    // Set the other settings...
    settings.setFlowControl(TNX::QPortSettings::FLOW_XONXOFF);
    settings.setParity(TNX::QPortSettings::PAR_NONE);
    settings.setDataBits(TNX::QPortSettings::DB_8);
    settings.setStopBits(TNX::QPortSettings::STOP_2);
    // ...then apply them to the port, and open it.
    return setPortSettings(settings) &&
        setCommTimeouts(CtScheme_TimedRead, 100) &&
        open(QIODevice::ReadWrite);       
}

QByteArray C45BSerialPort::readUntil(char terminator, qint64 maxSize)
{
    QByteArray data;
    qint64 bytesRead = 0;
    while (bytesRead < maxSize)
    {
        unsigned char c = 0;
        if (!getChar(reinterpret_cast<char*>(&c)))
            // Timeout
            break;
        if (c == terminator)
            break;
        data.append(c);
    }
    return data;
}

bool C45BSerialPort::downloadLine(QString s)
{
	// Send the hex record
    // if (m_verbose)
    //     cout << "Sending '" << s.trimmed().toAscii().data() << "'" << endl;
    write(s.toAscii());
	
	// read until XON, 10 characters or timeout
    Msleep(8);

    QByteArray r = readUntil(XON, 10);
    //cout << "REPLY " << QString(r).toLatin1().data() << endl;
    // The bootloader replies with '.' on success...
    if( r.contains('-') )
    {
        cout << "Something went wrong during programming " << endl;
        return false;
    }
    if (!r.contains('.') && !r.contains('*'))
    {
        if (m_verbose)
        {
            if (r.isEmpty())
                cout << "Timeout" << endl;
            else
                cout << "Reply: " << FormatControlChars(r).toStdString() << endl;
        }
        return false;
    }
    // ...and with '*' on page write
    if (m_verbose && r.contains('*'))
        cout << "+" << flush;
    return true;
}
