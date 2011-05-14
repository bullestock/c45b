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

	quint32 endAddress = 0x00000000;

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
        
		if (recordType == 2)
        {
            // extended segment address record
			extendedSegmentAddress = asciiToHex(line[9], line[10]) << 8;
			checkSum += (extendedSegmentAddress >> 8);  // chechsum...
			extendedSegmentAddress += asciiToHex(line[11], line[12]);
			checkSum += (extendedSegmentAddress & 0xff);  // chechsum...
		}
        else if(recordType == 1)
        {
            // end of file record
            if (m_verbose)
                cout << "EOF" << endl;
			break;
		}
        else if(recordType == 0)
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
			if (endAddress < address + (extendedSegmentAddress * 16U) + (i >> 1))
            {
				endAddress = address + (extendedSegmentAddress * 16U) + (i >> 1);
			}
			
			if (endAddress > (MAX_FLASH_BYTES/16*46))
            {
                m_lastError = QString("Overflow (address %1)").arg(endAddress);
				return false;
			}
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


// 	SetCtrlAttribute(tabMainPanelHandle, TABMAIN_LEDCONNECTBOOTLOADER, ATTR_LABEL_TEXT, "Uploading"); 
	
// 	// status message in communication log
// 	Fmt(tmpBuffer, "Uploading hex file to bootloader:\r\n");
// 	SetCtrlVal (panelLogHandle, LOGPANEL_COMMUNICATIONLOG, tmpBuffer);

	
// 	extendedSegmentAddress = 0x00000000;
// 	byteCount = 0x10;  // byte count is fixed to 16 bytes
// 	unsigned long m_bufferAddress = 0;  // we always start at bottom of flash
// 	unsigned char flagErrorUploading = 0;
	
	
// 	// outer loop for each 16 byte hex file lines
// 	for(unsigned short lineCounter = 0; (lineCounter <= ((endAddress-1) / 16)) && (flagErrorUploading == 0); ++lineCounter) {

// 		char i = 0;
// 		unsigned short hexFileDataByteCounter = 0;
// 		char *buffer = line;  // temporary buffer now points to hex line buffer
		
// 		m_bufferAddress = lineCounter * 16;
		
// 		// start printing normal data record
// 		i = sprintf(buffer, ":%02X%04X%02X", byteCount, (unsigned short)(m_bufferAddress & 0xffff), 0x00);
// 		buffer += i;

// 		// checksum...
// 		fileCheckSum = byteCount;
// 		fileCheckSum += (m_bufferAddress >> 8);
// 		fileCheckSum += (m_bufferAddress & 0xff);
		
// 		// inner loop for the 16 data bytes
// 		for(hexFileDataByteCounter = 0; hexFileDataByteCounter < 16; ++hexFileDataByteCounter) {
			
// 			unsigned char c = m_buffer[m_bufferAddress + hexFileDataByteCounter];
			
// 			i = sprintf(buffer, "%02X", c);
// 			buffer += i;
// 			fileCheckSum += c;
			
// 		}
		
// 		// append the calculated checksum and line ending
// 		sprintf(buffer, "%02X\n", (0x100 - fileCheckSum) & 0xff);

// 		// now line contains all the stuff printed
// 		ec = uploadHexLine(line);
// 		if(ec < 0) {
// 			flagErrorUploading = 1;
// 			break;
// 		};

		
		
// 		// check if we would exceed the 64k page address range with the next hex file line and insert a 
// 		if( ((m_bufferAddress & 0xffff) + 16) == 0x10000) {
// 			extendedSegmentAddress += 0x1000;  // the segment address is later multiplied by 16 and added to the hex record address
// 			sprintf(line, ":02000002%04X%02X\n", extendedSegmentAddress, (unsigned char)(0x100 - (4 + (extendedSegmentAddress >> 8) + (extendedSegmentAddress & 0xff))));

// 			if(uploadHexLine(line) < 0) {
// 				flagErrorUploading = 1;
// 				break;
// 			};
		
// #ifdef DEBUG
// 			printf("%s", line);
// #endif
// 		}
		
// 		// set progress bar
// 		ProgressBar_SetPercentage(tabMainPanelHandle, TABMAIN_PROGRESSBAR, lineCounter*100 / ((endAddress-1) / 16), "");
// 	}
	
// 	// if no error occured, we may append the end record
// 	if(flagErrorUploading == 0) {

// 		sprintf(line, ":00000001FF\n");
// 		if(uploadHexLine(line) < 0) {
// 			flagErrorUploading = 1;
// 		}
// 	}


    
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
