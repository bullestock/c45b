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

#include <QFile>

#include "hexfile.h"
#include "hexutils.h"


using namespace std;

// Currently the biggest AVR controller has 256k Flash
const quint32 MAX_FLASH_BYTES = 262144;


HexFile::HexFile()
{
    reset();
}

HexFile::~HexFile()
{
}

bool HexFile::load(QString fileName, bool verbose)
{
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_lastError = "File not found";
        return false;
    }
    reset();
    quint16 lineNr = 0;
    quint32 extendedSegmentAddress = 0;
    while (!f.atEnd())
    {
        ++lineNr;
        QByteArray line = f.readLine();

        // grab hexfile information
        unsigned char byteCount = asciiToHex(line[1], line[2]);  // get number of bytes
        unsigned char checkSum = byteCount;  // start checksum computation
        quint32 address = asciiToHex(line[3], line[4]);  // get address high byte
        checkSum += (unsigned char) address;  // checksum...
        address = address << 8;
        unsigned char low = asciiToHex(line[5], line[6]);  // get address low byte
        address += low;
        checkSum += (unsigned char) (address & 0xff);  // checksum...
        unsigned char recordType = asciiToHex(line[7], line[8]);  // get record type
        checkSum += recordType;
        unsigned char fileCheckSum = asciiToHex(line[(byteCount*2)+9], line[(byteCount*2)+10]);  // get the checksum

        switch (recordType)
        {
        case 2:
            // extended segment address record
            extendedSegmentAddress = asciiToHex(line[9], line[10]) << 8;
            checkSum += (extendedSegmentAddress >> 8);  // chechsum...
            extendedSegmentAddress += asciiToHex(line[11], line[12]);
            checkSum += (extendedSegmentAddress & 0xff);  // chechsum...
            if(verbose)
            {
                cout << "Got extended adress record" <<endl;
            }
            break;

        case 1:
            // end of file record
            if (verbose)
            {
                cout << "Loaded hex file" << endl;
                cout << "Read " << QByteArray::size() << " bytes" << endl;
            }
            break;

        case 0:
            {
                // data record
                for (unsigned int i = 0; i < (2 * ((unsigned int)byteCount)); i += 2)
                {
                    unsigned char dataByte = asciiToHex(line[i+9], line[i+10]);
                    checkSum += dataByte;  // compute checksum
                    if(!setByte(address + (extendedSegmentAddress * 16) + (i >> 1), dataByte))
                    {
                        m_lastError = QString("Maximum size exceeded");
                        return false;
                    }
                }

            }
            break;
        default:
            m_lastError = QString("Found unknown or unsupported record type (0x%1)").arg(recordType, 2, 16, QChar('0'));
            return false;
        }

        // check if checksum error
        if (((checkSum + fileCheckSum) & 0xff) != 0)
        {
            m_lastError = QString("Checksum error in line %1: Expected %2, computed %3").arg(lineNr).arg(fileCheckSum).arg(checkSum);
            return false;
        }
    }

    return true;
}

bool HexFile::setByte(quint32 address, quint8 data)
{
    if (address >= MAX_FLASH_BYTES)
    {
        m_lastError = QString("Overflow (address %1)").arg(address);
        return false;
    }
    if (address > static_cast<quint32>(QByteArray::size()))
        QByteArray::append(QByteArray(address - QByteArray::size(), 0));
    QByteArray::operator[](address) = data;
    return true;
}

bool HexFile::append(quint8 data)
{
    if (static_cast<quint32>(QByteArray::length()) >= MAX_FLASH_BYTES)
    {
        m_lastError = QString("Overflow (address %1)").arg(QByteArray::length()+1);
        return false;
    }
    QByteArray::append(data);
    return true;
}

void HexFile::reset()
{
    QByteArray::fill(0, MAX_FLASH_BYTES);
    QByteArray::clear();
}

bool HexFile::equal(const HexFile& other)
{
    if (size() != other.size())
        return false;

    for (int i = 0; i < size(); ++i)
        if (operator[](i) != other[i])
            return false;
    return true;
}

QStringList HexFile::getHexFile() const
{
    QStringList result;
    const QChar zeropad('0');
    const quint32 byteCount = 16;  // byte count is fixed to 16 bytes
    const quint32 hexSize = static_cast<quint32>(QByteArray::size());

    int lines = (hexSize+(byteCount-1))/byteCount; // round up

    for (int line = 0; line < lines; ++line)
    {
        quint32 address   = line*byteCount;
        quint16 addressSh = address;

        // preamble

        if(    (address>0)                          // new segment
            && ((address% (0x10000))==0))
        {
            quint16 segAddress = address/16;
            quint8 segChksum = ((2+2+(segAddress >> 8) + (segAddress & 0xFF)) ^ 0xFF) +1;
            QString segStr = QString(":02000002%1%2\n")
                                .arg(segAddress, 4, 16, zeropad)
                                .arg(segChksum, 2, 16, zeropad);
            result.append(segStr);

        }
        quint32 thisLinesBytecount = (address+byteCount) > hexSize
                                        ? hexSize-address
                                        : byteCount;
        QString str = QString(":%1%2%3").arg(thisLinesBytecount, 2,16, zeropad)
                                        .arg(addressSh, 4, 16, zeropad)
                                        .arg(0, 2, 16, zeropad);
        quint8 checksum = thisLinesBytecount + (addressSh >> 8) + (addressSh & 0xFF);
        for (quint32 i = 0; i < thisLinesBytecount; ++i)
        {
            quint8 byte = QByteArray::operator[](address + i);
            checksum += byte;
            str += QString("%1").arg(byte, 2, 16, zeropad);
        }
        checksum = (checksum ^ 0xFF)+1;
        str += QString("%1\n").arg(checksum, 2, 16, zeropad);
        result.append(str);
    }
    result.append(QString(":00000001FF\n"));

    return result;
}
