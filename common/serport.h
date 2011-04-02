#include <qextserialport.h>

class C45BSerialPort : public QextSerialPort
{
public:
    C45BSerialPort(QString device, QextSerialPort::QueryMode mode);

    ~C45BSerialPort();

    bool init();

    /// Read until a character equal to c has been read, or until maxSize characters have been read.
    /// The c character is not included in the returned data.
    QByteArray readUntil(char c, qint64 maxSize);
};

    
