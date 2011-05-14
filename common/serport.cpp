#include <iostream>
#include <iomanip>

#include "serport.h"

using namespace std;

C45BSerialPort::C45BSerialPort(QString device)
    : QSerialPort(device)
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
        setCommTimeouts(CtScheme_TimedRead, 1000) &&
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
