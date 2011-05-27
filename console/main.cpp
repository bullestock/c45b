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

#include "hexfile.h"
#include "platform.h"
#include "serport.h"

using namespace std;


static void SilentMsgHandler(QtMsgType, const char *);


static const char* version = "0.2";


void Usage(ez::ezOptionParser& opt)
{
    string usage;
    opt.getUsage(usage, 80, ez::ezOptionParser::ALIGN);
    cout << usage;
};

ostream& operator<<(ostream& os, const QString& s)
{
    os << s.toLatin1().data();
    return os;
}

int main(int argc, char** argv)
{
    // How many ms to wait for bootloader prompt
    const int InitialTimeOut = 60000;

    // Suppress qDebug output from QSerialPort
    qInstallMsgHandler(SilentMsgHandler);
    
    QCoreApplication app(argc, argv);

    ez::ezOptionParser opt;

    opt.overview = "Tool for communicating with the Chip45 bootloader";
    opt.syntax = "c45b [OPTIONS] hexfile";
    opt.example = "c45b -f -p /dev/ttyUSB0 avrblink.hex\n";

    opt.add("", false, 0, 0, "Show version", "-v");
    opt.add("", true,  1, 0, "Serial port", "-p");
    opt.add("", false, 1, 0, "Baud rate", "-b");
    opt.add("", false, 0, 0, "Program flash memory", "-f");
    opt.add("", false, 0, 0, "Program EEPROM", "-e");
    opt.add("", false, 0, 0, "Show debug info", "-d");
    opt.add("", false, 0, 0, "Be verbose", "--verbose");
    opt.add("", false, 0, 0, "Show help", "-h", "--help");

    opt.parse(argc, const_cast<const char**>(argv));

    if (opt.isSet("-h"))
    {
        Usage(opt);
        return 0;
    }

    if (opt.isSet("-v"))
    {
        cout << "c45b version " << version << endl
             << "Copyright 2011 Torsten Martinsen (http://bullestock.net)"
             << endl;
        return 1;
    }

    const bool doFlash = opt.isSet("-f");
    const bool doEeprom = opt.isSet("-e");
    if (!doFlash && !doEeprom)
    {
        cout << "Neither -f nor -e specified - nothing to do" << endl;
        return 1;
    }
    if (opt.lastArgs.size() < 1)
    {
        cout << "No filename specified" << endl;
        return 1;
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

    bool debug = opt.isSet("-d");
    bool verbose = debug || opt.isSet("-verbose");

    QString fileName(opt.lastArgs[0]->c_str());
    HexFile hexFile(fileName, verbose);
    if (!hexFile.load())
    {
        cout << "Failed to load file '" << fileName << "': " << hexFile.errorString() << endl;
        return 1;
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

    if (verbose)
        cout << "Connecting..." << flush;

    QTime t;
    t.start();
    while (!port->bytesAvailable() && (t.elapsed() < InitialTimeOut))
    {
        // "After a reset the bootloader waits for approximately 2 seconds to detect a
        //  transmission at its RXD pin. If so, it will measure the timing of the rising
        //  and falling edges of four consecutive characters 'U' at the host's baud to
        //  determine its correct baud rate prescaler."

        port->putChar('U');
        port->putChar('U');
        port->putChar('U');
        port->putChar('U');
        port->flushOutBuffer();

        Msleep(100);
        if (verbose && (t.elapsed() > 1000))
        {
            cout << "." << flush;
            t.start();
        }
    }
    QString prompt = port->readUntil('>', 20);

    if (debug)
        cout << "Read: " << prompt << endl;
    if (verbose)
        cout << "\rConnected                                                                      " << endl;
    
    if (prompt.isEmpty())
    {
        cout << "Error: No initial reply from bootloader" << endl;
        return 1;
    }
    if (!prompt.startsWith("c45b2"))
    {
        cout << "Error: Wrong bootloader version: " << prompt << endl;
        return 1;
    }
    if (verbose)
        cout << "Bootloader " << prompt.mid(5).simplified() << endl;

    // Flush
    port->readAll();
    Msleep(10);

    port->putChar('\n');
    Msleep(100);
    port->readAll();

    QString cmd(doFlash ? "pf" : "pe");
    port->write(cmd.toAscii().data(), cmd.size());
    port->putChar('\n');
    // Wait for "pf+\r"
    QString reply = port->readUntil('\r', 10);
    reply = reply.trimmed();
    QString expected = cmd + QString("+");
    if (!reply.startsWith(expected))
    {
        cout << "Error: Bootloader did not respond to '" << (doFlash ? "pf" : "pe") << "' command" << endl;
        if (verbose)
            cout << "Reply: " << reply << endl;
        return 1;
    }

    // Send to bootloader
    if (verbose)
        cout << "Programming " << (doFlash ? "flash" : "EEPROM") << " memory..." << flush;

    QString line;
    while (hexFile.getLine(line))
        if (!port->downloadLine(line))
        {
            cout << "Error: Failed to download line " << hexFile.currentLine() << endl;
            return 1;
        }

    if (!port->downloadLine(":00000001FF\n"))
    {
        cout << "Error: Failed to complete download" << endl;
        return 1;
    }

    if (verbose)
        cout << "...done" << endl;

    port->close();
}

static void SilentMsgHandler(QtMsgType, const char *)
{
}
