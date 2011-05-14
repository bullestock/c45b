#include <QByteArray>
#include <QString>

class HexFile
{
public:
    HexFile(QString fileName, bool verbose);

    ~HexFile();

    bool load();

    QString errorString() const;
    
private:
    static unsigned char asciiToHex(unsigned char a);
    static unsigned char asciiToHex(unsigned char high, unsigned char low);

    QString m_fileName;
    bool m_verbose;
    QByteArray m_buffer;
    QString m_lastError;
};

    
