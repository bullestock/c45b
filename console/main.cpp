// Copyright 2010 Torsten Martinsen <bullestock@bullestock.net>

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

#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include "hexfile.h"
#include "serport.h"

using namespace std;


static void SilentMsgHandler(QtMsgType, const char *);

bool waitFor(C45BSerialPort* port, char c, QString s);


static const char* version = "0.1";


int main(int argc, char *argv[])
{
    const int InitialTimeOut = 60000;

    // Suppress qDebug output from QSerialPort
    qInstallMsgHandler(SilentMsgHandler);
    
    QCoreApplication app(argc, argv);

    QTextStream cout(stdout, QIODevice::WriteOnly);

    KAboutData about("c45b", "", ki18n("c45b"), version, 
                     ki18n("Tool for communicating with the Chip45 bootloader"),
                     KAboutData::License_GPL_V3,
                     ki18n("Copyright 2011 Torsten Martinsen"),
                     ki18n(""),
                     "http://bullestock.net",
                     "torsten@bullestock.net");
    about.addAuthor(ki18n("Torsten Martinsen"));
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("p <port>", ki18n("Serial port"));
    options.add("f", ki18n("Program flash memory"));
    options.add("e", ki18n("Program EEPROM"));
    options.add("d", ki18n("Show debug info"));
    options.add("verbose", ki18n("Verbose"));
    options.add("help", ki18n("Help"));
    options.add("+file", ki18n("Name of the hex file"));

    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if (!args->isSet("f") && !args->isSet("e"))
    {
        cout << "Neither -f nor -e specified - nothing to do" << endl;
        return 1;
    }
    if (args->count() < 1)
    {
        cout << "No filename specified" << endl;
        return 1;
    }

    bool debug = args->isSet("d");
    bool verbose = debug || args->isSet("verbose");

    QString fileName(args->arg(0));
    HexFile hexFile(fileName, verbose);
    if (hexFile.load())
    {
        cout << "Failed to load file '" << fileName << "': " << hexFile.errorString() << endl;
        return 1;
    }
    
    QString device = args->getOption("p");

    C45BSerialPort* port = new C45BSerialPort(device);
    if (!port->init())
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

        usleep(100000);
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
    usleep(100000);

    if (args->isSet("f"))
    {
        port->putChar('\n');
        usleep(100000);
        QByteArray r = port->readAll();
        port->write("pf\n", 311);
        // Wait for "pf+\r"
        QString reply = port->readUntil('\r', 10);
        reply = reply.remove(QChar(17)).remove(QChar(19)).trimmed();
        if (!reply.startsWith("pf+"))
        {
            cout << "Error: Bootloader did not respond to 'pf' command" << endl;
            if (verbose)
                cout << "Reply: " << reply << endl;
            return 1;
        }
        // Load hex file into memory

        // Send to bootloader
        cout << "Programming flash memory.." << flush;
        
        for (int i = 0; i < 10; ++i)
        {
            cout << "." << flush;
            usleep(200000);
        }
        cout << "done" << endl;
    }

    port->close();
}

static void SilentMsgHandler(QtMsgType, const char *)
{
}

bool waitFor(C45BSerialPort* port, char c, QString s)
{
    char got = 0;
    if (!port->getChar(&got))
    {
        cout << "Error: Timeout waiting for " << qPrintable(s) << endl;
        return false;
    }
    if (got != c)
    {
        cout << "Error: Expected " << qPrintable(s) << ", got " << static_cast<int>(got) << endl;
        return false;
    }
    return true;
}
