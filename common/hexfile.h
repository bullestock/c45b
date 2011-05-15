#include <QByteArray>
#include <QString>

class HexFile
{
public:
    HexFile(QString fileName, bool verbose);

    ~HexFile();

    bool load();

    QString errorString() const;

    int currentLine() const;

    void reset();

    bool getLine(QString& s);

private:
    static unsigned char asciiToHex(unsigned char a);
    static unsigned char asciiToHex(unsigned char high, unsigned char low);

    QString m_fileName;
    bool m_verbose;
    QByteArray m_buffer;
    QString m_lastError;

    quint32 m_endAddress;
    quint16 m_extendedSegmentAddress;
    quint16 m_currentLine;
};

    
