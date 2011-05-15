#include <iostream>
#include <iomanip>

#include <QFile>

#include "hexfile.h"


using namespace std;

// Currently the biggest AVR controller has 256k Flash
const int MAX_FLASH_BYTES = 262144;


HexFile::HexFile(QString fileName, bool verbose)
    : m_fileName(fileName),
      m_verbose(verbose)
{
    reset();
}

HexFile::~HexFile()
{
}

bool HexFile::load()
{
    QFile f(m_fileName);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        m_lastError = "File not found";
        return false;
    }

    m_buffer.resize(MAX_FLASH_BYTES/16*46);

	m_endAddress = 0x00000000;

    while (!f.atEnd())
    {
        QByteArray line = f.readLine();

		// grab hexfile information
		unsigned char byteCount = asciiToHex(line[1], line[2]);  // get number of bytes
		unsigned char checkSum = byteCount;  // start checksum computation
		quint16 address = asciiToHex(line[3], line[4]);  // get address high byte
		checkSum += (unsigned char) address;  // checksum...
		address = (address << 8) + asciiToHex(line[5], line[6]);  // get address low byte   
		checkSum += (unsigned char) (address & 0xff);  // checksum...
		unsigned char recordType = asciiToHex(line[7], line[8]);  // get record type
		checkSum += recordType;
		unsigned char fileCheckSum = asciiToHex(line[(byteCount*2)+9], line[(byteCount*2)+10]);  // get the checksum

        quint16 extendedSegmentAddress = 0;
        
		switch (recordType)
        {
        case 2:
            // extended segment address record
			extendedSegmentAddress = asciiToHex(line[9], line[10]) << 8;
			checkSum += (extendedSegmentAddress >> 8);  // chechsum...
			extendedSegmentAddress += asciiToHex(line[11], line[12]);
			checkSum += (extendedSegmentAddress & 0xff);  // chechsum...
            break;

        case 1:
            // end of file record
            if (m_verbose)
                cout << "Loaded hex file" << endl;
			break;

        case 0:
        {
            // data record
            unsigned int i = 0;
			for ( ; i < (2 * byteCount); i += 2)
            {
				unsigned char dataByte = asciiToHex(line[i+9], line[i+10]);
				checkSum += dataByte;  // compute checksum
				m_buffer[address + (extendedSegmentAddress * 16) + (i >> 1)] = dataByte;
			}
			
			// in case the hex file is unordered
			if (m_endAddress < address + (extendedSegmentAddress * 16U) + (i >> 1))
            {
				m_endAddress = address + (extendedSegmentAddress * 16U) + (i >> 1);
			}
			
			if (m_endAddress > (MAX_FLASH_BYTES/16*46))
            {
                m_lastError = QString("Overflow (address %1)").arg(m_endAddress);
				return false;
			}
		}
        break;
        }

		// check if checksum error
		if (((checkSum + fileCheckSum) & 0xff) != 0)
        {
            m_lastError = QString("Checksum error: Expected %1, computed %2").arg(fileCheckSum).arg(checkSum);
            return false;
		}
    }
    return true;
}

QString HexFile::errorString() const
{
    return m_lastError;
}

int HexFile::currentLine() const
{
    return m_currentLine;
}

void HexFile::reset()
{
	m_extendedSegmentAddress = 0x00000000;
    m_currentLine = 0;
}

bool HexFile::getLine(QString& s)
{
	const int byteCount = 0x10;  // byte count is fixed to 16 bytes
	
	// outer loop for each 16 byte hex file lines
	if (m_currentLine > ((m_endAddress-1) / 16))
        return false;

    quint32 bufferAddress = m_currentLine * 16;

    const QChar fill = '0';
    s = QString(":%1%2%3").arg(byteCount, 2, 16, fill).arg(bufferAddress & 0XFFFF, 4, 16, fill).arg(0, 2, 16, fill);

    // checksum
    unsigned char fileCheckSum = byteCount;
    fileCheckSum += (bufferAddress >> 8);
    fileCheckSum += (bufferAddress & 0xff);

    s += QString(m_buffer.mid(bufferAddress, 16).toHex()).toUpper();
    for (int i = 0; i < 16; ++i)
    {
        unsigned char c = m_buffer[bufferAddress + i];
        fileCheckSum += c;
    }

    s += QString("%1\n").arg(static_cast<unsigned char>(256 - fileCheckSum), 2, 16, fill);
		
    // check if we would exceed the 64k page address range with the next hex file line
    if (((bufferAddress & 0xffff) + 16) == 0x10000)
    {
        m_extendedSegmentAddress += 0x1000;  // the segment address is later multiplied by 16 and added to the hex record address
        s += QString(":02000002%1%2\n").
            arg(m_extendedSegmentAddress, 4, 16, fill).
            arg(static_cast<unsigned char>(0x100 - (4 + (m_extendedSegmentAddress >> 8) + (m_extendedSegmentAddress & 0xFF))), 2, 16, fill);
	}
	
    ++m_currentLine;

    return true;
}

    
unsigned char HexFile::asciiToHex(unsigned char a)
{
	if(a >= 'a')
		return a - 'a' + 10;

    if(a >= 'A')
		return a - 'A' + 10;
    
	if (a >= '0')
		return a - '0';
	
    return 0;
}


unsigned char HexFile::asciiToHex(unsigned char high, unsigned char low)
{
	return (asciiToHex(high) << 4) + asciiToHex(low);
}
