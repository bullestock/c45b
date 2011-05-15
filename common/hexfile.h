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

#include <QByteArray>
#include <QString>

class HexFile
{
public:
    HexFile(QString fileName, bool verbose);

    ~HexFile();

    bool load();

    QString errorString() const;

    int currentLine() const;

    void reset();

    bool getLine(QString& s);

private:
    static unsigned char asciiToHex(unsigned char a);
    static unsigned char asciiToHex(unsigned char high, unsigned char low);

    QString m_fileName;
    bool m_verbose;
    QByteArray m_buffer;
    QString m_lastError;

    quint32 m_endAddress;
    quint16 m_extendedSegmentAddress;
    quint16 m_currentLine;
};

    
