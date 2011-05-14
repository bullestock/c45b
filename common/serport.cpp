#include <iostream>
#include <iomanip>

#include "serport.h"

using namespace std;

C45BSerialPort::C45BSerialPort(QString device)
{
    setDeviceName(device);
}

C45BSerialPort::~C45BSerialPort()
{
}

bool C45BSerialPort::init()
{
    bool ok = open(QIODevice::ReadWrite) &&
        setBaudRate(BaudRate19200) &&
        setFlowControl(FlowControlXonXoff) &&
        setParity(ParityNone) &&
        setDataBits(DataBits8) &&
        setStopBits(StopBits2);
    setTotalReadConstantTimeout(1000);
	//setTimeout(-1);
    return ok;    
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
