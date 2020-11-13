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

#include <QtSerialPort/qserialport.h>

class C45BSerialPort : public QSerialPort
{
public:
    static const char XON  = 0x11;
    static const char XOFF = 0x13;

    C45BSerialPort(QString device,
                   bool verbose);

    ~C45BSerialPort();

    /// 0: Use default
    bool init(int baudRate = 0);

    /// Read until a character equal to c has been read, or until maxSize characters have been read.
    /// The c character is not included in the returned data.
    QByteArray readUntil(char c, qint64 maxSize);

    bool downloadLine(QString s);

private:
    bool m_verbose;
};

    
