#include <QtSerialPort/qserialport.h>

class C45BSerialPort : public TNX::QSerialPort
{
public:
    C45BSerialPort(QString device,
                   bool verbose);

    ~C45BSerialPort();

    bool init();

    /// Read until a character equal to c has been read, or until maxSize characters have been read.
    /// The c character is not included in the returned data.
    QByteArray readUntil(char c, qint64 maxSize);

    bool downloadLine(QString s);

private:
    bool m_verbose;
};

    
