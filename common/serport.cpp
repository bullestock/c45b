#include <iostream>
#include <iomanip>

#include "serport.h"

using namespace std;

C45BSerialPort::C45BSerialPort(QString device, QextSerialPort::QueryMode mode)
    : QextSerialPort(device, mode)
{
}

C45BSerialPort::~C45BSerialPort()
{
}

bool C45BSerialPort::init()
{
//    setBaudRate(BAUD19200);
        setBaudRate(BAUD9600);
	setFlowControl(FLOW_XONXOFF);
	setParity(PAR_NONE);
	setDataBits(DATA_8);
	setStopBits(STOP_2);
	setTimeout(1000);
//	setTimeout(-1);
    return open(QIODevice::ReadWrite);
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
        cout << "Got " << unsigned(c) << " (" << c << ")" << endl;
        data.append(c);
    }
    return data;
}
