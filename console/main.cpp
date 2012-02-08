// Copyright 2011 Torsten Martinsen <bullestock@bullestock.net>

// This file is part of c45b.

// c45b is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// c45b is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with c45b.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include <iomanip>

#include <errno.h>

#include <QApplication>
#include <QDebug>
#include <QTextStream>
#include <QThread>
#include <QTime>

#include <ezOptionParser.hpp>

#include "c45butils.h"
#include "hexfile.h"
#include "hexfiletester.h"
#include "hexutils.h"
#include "platform.h"
#include "serport.h"

using namespace std;


static void SilentMsgHandler(QtMsgType, const char *);


static const char* version = "0.3";


void Usage(ez::ezOptionParser& opt)
{
    string usage;
    // Limit to 79 chars to avoid line wrapping on Win32
    opt.getUsage(usage, 79, ez::ezOptionParser::ALIGN);
    cout << usage;
};

ostream& operator<<(ostream& os, const QString& s)
{
    os << s.toLatin1().data();
    return os;
}

bool parseAppCmd(const std::string& i_str, QByteArray& o_data)
{
    const std::string specialCharsSym("tnr\\");
    const std::string specialChars("\t\n\r\\");
    const std::string hexChars("0123456789abcdefABCDEF");
    const char hexVals[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 10, 11, 12, 13, 14, 15 };

    string::const_iterator it = i_str.begin();
    string::const_iterator end = i_str.end();

    for( ; it != end; ++it)
    {
        char c = *it;
        if (c == '\\')
        {
            ++it;
            if (it == end)
                return false;
            c = *it;
            string::size_type pos = specialCharsSym.find(c);
            if (pos != specialCharsSym.npos)
                o_data.push_back(specialChars[pos]);
            else
            {
                char databyte = 0;
                pos = hexChars.find(c);
                if (pos == hexChars.npos)
                    return false;
                databyte = hexVals[pos] << 4;
                ++it;
                if (it == end)
                    return false;
                c = *it;
                pos = hexChars.find(c);
                if (pos == hexChars.npos)
                    return false;
                databyte |= hexVals[pos];
                o_data.push_back(databyte);
            }
        }
        else
            o_data.push_back(c);
    }
    return true;
}


bool readEeprom(HexFile& o_hexFile, quint32 eepromReadBytes, C45BSerialPort* port, bool verbose)
{
    for (quint32 i = 0; i < eepromReadBytes; ++i)
    {
        QString cmd = QString("er%1").arg(i, 4, 16, QChar('0'));
        port->write(cmd.toAscii());
        port->putChar('\n');
        QString reply = port->readUntil('\r', 10);
        reply = reply.replace(QChar(C45BSerialPort::XOFF), "").trimmed();
        QString expected = cmd + QString("+");
        if (!reply.startsWith(expected))
        {
            cout << "Error: Bootloader did not respond to 'er' command" << endl;
            if (verbose)
                cout << "Reply: " << FormatControlChars(reply) << endl;
            return false;
        }
        reply = port->readUntil('\r', 10);
        reply = reply.trimmed();
        if (verbose)
            cout <<reply<<" ";
        o_hexFile.append(reply.toInt(0, 16));
        port->readUntil(C45BSerialPort::XON, 10);
    }
    if (verbose)
        cout <<endl;
    return true;
}

bool program(const HexFile& hexFile, C45BSerialPort* port, int delay, bool doFlash, bool verbose)
{
    QString cmd(doFlash ? "pf" : "pe");
    port->write(cmd.toAscii().data(), cmd.size());
    port->putChar('\n');
    // Wait for "pf+\r"
    QString reply = port->readUntil('\r', 10);
    reply = reply.replace(QChar(0x13), "").trimmed();
    QString expected = cmd + QString("+");
    if (!reply.startsWith(expected))
    {
        cout << "Error: Bootloader did not respond to '" << (doFlash ? "pf" : "pe") << "' command" << endl;
        if (verbose)
            cout << "Reply: " << FormatControlChars(reply) << endl;
        return false;
    }

    // Send to bootloader
    if (verbose)
        cout << "Programming " << (doFlash ? "flash" : "EEPROM") << " memory..." << flush;

    port->readAll();

    QStringList hexFileLines = hexFile.getHexFile();
    quint32 lineNr = 0;
    foreach(QString line, hexFileLines)
    {
        ++lineNr;
        if (!port->downloadLine(line))
        {
            cout << "Error: Failed to download line " << lineNr << endl;
            return false;
        }
        if(delay > 0)
            Msleep(delay);
    }

    QByteArray received = port->readAll();

    if (received.contains('-'))
    {
        cout << "Something went wrong during programming" << endl
             << "Reply: " << FormatControlChars(received) << endl;
        return false;
    }
    if (verbose)
        cout << "...done" << endl;

    return true;
}

bool connectBootloader(C45BSerialPort* port, bool debug, bool verbose)
{
    // How many ms to wait for bootloader prompt
    const int InitialTimeOut = 60000;

    QTime t;
    t.start();
    QTime t2;
    t2.start();
    int avail = 0;

    QString prompt;
    bool connected = false;
    bool gotActiveBootloader = false;
    while (!connected && (t.elapsed() < InitialTimeOut))
    {
        // "After a reset the bootloader waits for approximately 2 seconds to detect a
        //  transmission at its RXD pin. If so, it will measure the timing of the rising
        //  and falling edges of four consecutive characters 'U' at the host's baud to
        //  determine its correct baud rate prescaler."

        port->putChar('U');
        port->putChar('U');
        port->putChar('U');
        port->putChar('U');
        port->putChar('\n');
        port->flushOutBuffer();

        Msleep(100);
        if (verbose && (t2.elapsed() > 1000))
        {
            cout << "." << flush;
            t2.start();
        }
        avail = port->bytesAvailable();
        if(avail)
        {
            prompt = port->readUntil(C45BSerialPort::XON, 30);
            if (prompt.contains("c45b2"))
            {
                connected = true;
                if(debug)
                    cout << "Found fresh bootloader" << endl;
            }
            else if (prompt.contains(QString("%1-\n\r>").arg(QChar(C45BSerialPort::XOFF))))
            {
                connected = true;
                gotActiveBootloader = true;
                if(debug)
                    cout << "Found already activated bootloader" << endl;
            }
        }
    }

    if (debug)
        cout << "Read " << prompt.size() << " bytes: " << FormatControlChars(prompt).toStdString() << endl;
    if (verbose)
        cout << "\rConnected                                                                      " << endl;

    if (prompt.isEmpty())
    {
        cout << "Error: No initial reply from bootloader" << endl;
        return false;
    }
    if (!gotActiveBootloader && !prompt.contains("c45b2"))
    {
        cout << "Error: Wrong bootloader version: " << prompt << endl;
        return false;
    }

    if (gotActiveBootloader)
        cout << "Warning: bootloader was already active - could not check for compatible version" << endl;
    else if (verbose)
        cout << "Bootloader " << prompt.mid(5).simplified() << endl;

    // Flush
    port->readAll();
    Msleep(10);

    port->putChar('\n');
    Msleep(100);
    port->readAll();
    return true;
}


int main(int argc, char** argv)
{
    // Suppress qDebug output from QSerialPort
    qInstallMsgHandler(SilentMsgHandler);
    
    QCoreApplication app(argc, argv);

    ez::ezOptionParser opt;

    opt.overview = "Tool for communicating with the Chip45 bootloader";
    opt.syntax = "c45b [OPTIONS]";
#ifdef WIN32
    opt.example = "c45b -p COM3 -b 57600 -c \"R!\" -f avrblink.hex -e avrblink.eep -r\n";
#else
    opt.example = "c45b -p /dev/ttyUSB0 -b 57600  -c \"R!\" -f avrblink.hex -e avrblink.eep -r\n";
#endif

    opt.add("", false, 0, 0, "Show version", "-v", "--version");
    opt.add("", true,  1, 0, "Serial port"
#ifdef WIN32
            " (without colon)"
#endif
            , "-p", "--port");
    opt.add("", false, 1, 0, "Baud rate",                                    "-b", "--baud");
    opt.add("", false, 1, 0, "Program flash memory file",                    "-f", "--flash");
    opt.add("", false, 1, 0, "Program EEPROM file",                          "-e", "--eeprom");
    opt.add("", false, 1, 0, "Delay (in ms) to wait between sending "
                             "two lines of EEPROM data.\n"
                             "Set or increase this if writing EEPROM fails.","-ed", "--eepromdelay");
    opt.add("", false, 2,',',"Read EEPROM from device\n"
                             "Usage: -er destination.hex,length_in_bytes\n", "-er", "--eepromread");
    opt.add("", false, 0, 0, "Start application/leave bootloader on exit", "-r", "--runapp");
    opt.add("", false, 0, 0, "Show debug info",                              "-d", "--debug");
    opt.add("", false, 0, 0, "Be verbose",                                   "--verbose");
    opt.add("", false, 0, 0, "Show help",                                    "-h", "--help");
    opt.add("", false, 1, 0, "Command that should be sent to device before "
                             "the attempts to enter the bootloader.\n"
                             "Embed the command in double quotes and escape "
                             "special characters with backslash and two hex "
                             "digits. For example: \\07\\22 for bell and "
                             "double quote character. "
                             "The following escape sequences may be used:\n"
                             "\\\\ \\t \\n \\r ",                            "-c", "--appcmd");
    opt.add("", false, 1, 0, "Executes the hexfile implementation test with given file", "--testhex");
    opt.add("", false, 2,',',"Read the input hex file and write a "
                             "reformatted output hex file.\n"
                             "Usage: --reformathex input,output",            "--reformathex");

    opt.parse(argc, const_cast<const char**>(argv));

    bool debug = opt.isSet("-d");
    bool verbose = debug || opt.isSet("--verbose");

    if (opt.isSet("-h"))
    {
        Usage(opt);
        return 0;
    }

    if (opt.isSet("-v"))
    {
        cout << "c45b version " << version << endl
             << "Copyright 2012 Torsten Martinsen (http://bullestock.net)"
             << endl;
        return 1;
    }

    if (opt.isSet("--testhex"))  // check hexfiles prior to doing COM stuff
    {
        std::string fileName;
        opt.get("--testhex")->getString(fileName);
        HexFileTester tester;
        tester.test(QString::fromStdString(fileName));
        return 0;
    }
    if(opt.isSet("--reformathex"))
    {
        std::vector<std::string> str;
        opt.get("--reformathex")->getStrings(str);
        QString infile  = QString::fromStdString(str[0]);
        QString outfile = QString::fromStdString(str[1]);
        HexFile hex;
        if (!hex.load(infile, verbose))
        {
            cout << "Failed to load file '" << infile << "': " << hex.errorString() << endl;
            return 1;
        }
        if (!writeHexfile(outfile, hex))
        {
            cout << "Failed to write file '" << outfile << "' " << endl;
            return 1;
        }
        return 0;
    }

    vector<string> badOptions;
    if (!opt.gotRequired(badOptions))
    {
        for (size_t i = 0; i < badOptions.size(); ++i)
            cout << "ERROR: Missing required option " << badOptions[i] << ".\n\n";
        Usage(opt);
        return 1;
    }
    if (!opt.gotExpected(badOptions))
    {
        for (size_t i = 0; i < badOptions.size(); ++i)
            cout << "ERROR: Got unexpected number of arguments for option " << badOptions[i] << ".\n\n";
        Usage(opt);
        return 1;
    }

    const bool doFlash = opt.isSet("-f");
    const bool doEeprom = opt.isSet("-e");
    const bool doEepromRead = opt.isSet("-er");
    const bool runApp = opt.isSet("-r");
    bool sendAppCmd = opt.isSet("-c");

    if (!doFlash && !doEeprom && !doEepromRead)
    {
        cout << "Neither -f nor -e nor -er specified - nothing to do" << endl;
        if (sendAppCmd)  // starting without further options can be used to halt or reset device
        {                // but only if running application device reacts on app command.
            if(runApp)
                cout << "Just resetting device" << endl;
            else
                cout << "Just entering bootloader" << endl;
        }
        else
            return 1;
    }

    if( (doFlash || doEeprom) && doEepromRead)
    {
        cout << "You may only specify read or write commands." << endl;
        return 1;
    }

    HexFile flashHexFile;
    if (doFlash)  // check hexfiles prior to doing COM stuff
    {
        std::string fileName;
        opt.get("-f")->getString(fileName);
        if (!flashHexFile.load(fileName.c_str(), verbose))
        {
            cout << "Failed to load file '" << fileName << "': " << flashHexFile.errorString() << endl;
            return 1;
        }
    }

    int eepromWriteDelay = 0;
    HexFile eepHexFile;
    if (doEeprom)  // check hexfiles prior to doing COM stuff
    {
        std::string fileName;
        opt.get("-e")->getString(fileName);
        if (!eepHexFile.load(fileName.c_str(), verbose))
        {
            cout << "Failed to load file '" << fileName << "': " << eepHexFile.errorString() << endl;
            return 1;
        }
        if (opt.isSet("-ed"))
            opt.get("-ed")->getInt(eepromWriteDelay);
    }

    QString eepromReadFilename;
    quint32 eepromReadBytes = 0;
    if (doEepromRead)
    {
        std::vector<std::string> str;
        opt.get("-er")->getStrings(str);
        eepromReadFilename = QString::fromStdString(str[0]);
        QString bytes(str[1].c_str());
        bool correctNumber;
        eepromReadBytes = bytes.toUInt(&correctNumber);
        if (!correctNumber)
        {
            cout << bytes.toLocal8Bit().data() << " is not a valid number"<<endl;
            return 1;
        }
    }

    QByteArray appCmd;
    if (sendAppCmd)
    {
        std::string cmd;
        opt.get("-c")->getString(cmd);
        if(!parseAppCmd(cmd, appCmd))
        {
            cout << "Could not parse application command. Please check format and escape sequences" << endl;
            return 1;
        }
    }


    string s;
    opt.get("-p")->getString(s);
    QString device = s.c_str();

    C45BSerialPort* port = new C45BSerialPort(device, verbose);
    int baudRate = 0;
    opt.get("-b")->getInt(baudRate);
    if (!port->init(baudRate))
    {
        cout << "Error: Cannot open port '" << device << "': " << strerror(errno) << endl;
        return 1;
    }

    if (sendAppCmd)
    {
        if (verbose)
            cout << "Sending app command" << endl;
        port->write(appCmd);
    }

    if (verbose)
        cout << "Connecting..." << flush;

    if (!connectBootloader(port, debug, verbose))
        return 1;

    if(doFlash)
    {
        if (!program(flashHexFile, port, 0, true, verbose))
            return 1;
    }

    if (doEeprom)
    {
        if(!program(eepHexFile, port, eepromWriteDelay, false, verbose))
            return 1;
    }

    if (doEepromRead)
    {
        HexFile resultHexFile;
        if (!readEeprom(resultHexFile, eepromReadBytes, port, verbose))
            return 1;
        writeHexfile(eepromReadFilename, resultHexFile);
    }

    if (runApp)
    {
        port->putChar('g');
        port->putChar('\n');
    }

    port->close();
}

static void SilentMsgHandler(QtMsgType, const char *)
{
}
