#include <iostream>
#include <iomanip>

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

bool C45BSerialPort::init()
{
    return setBaudRate(TNX::QPortSettings::BAUDR_19200) &&
        setFlowControl(TNX::QPortSettings::FLOW_XONXOFF) &&
        setParity(TNX::QPortSettings::PAR_NONE) &&
        setDataBits(TNX::QPortSettings::DB_8) &&
        setStopBits(TNX::QPortSettings::STOP_2) &&
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
    usleep(8000);
    QByteArray r = readUntil(17, 10);
    // The bootloader replies with '.' on success...
    if (!r.contains('.'))
        return false;
    // ...and with '*' on page write
    if (m_verbose && r.contains('*'))
        cout << "+" << flush;
    return true;
}
