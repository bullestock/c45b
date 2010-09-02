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

#include <QApplication>
#include <QTextStream>
#include <QThread>
#include <QTime>

#include <qextserialport.h>

#include <kaboutdata.h>
#include <kcmdlineargs.h>


using namespace std;


static void SilentMsgHandler(QtMsgType, const char *);


static const char* version = "0.1";


int main(int argc, char *argv[])
{
    const int InitialTimeOut = 60000;

    // Suppress qDebug output from QextSerialPort
    qInstallMsgHandler(SilentMsgHandler);
        
    QCoreApplication app(argc, argv);

    QTextStream cout(stdout, QIODevice::WriteOnly);

    KAboutData about("c45b", 0, ki18n("c45b"), version,
                     ki18n("Tool for communicating with the Chip45 bootloader"),
                     KAboutData::License_GPL_V3,
                     ki18n("Copyright 2010 Torsten Martinsen"),
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
    options.add("help", ki18n("Help"));
    
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();
    if (!args->isSet("f") && !args->isSet("e"))
    {
        cout << "Neither -f nor -e specified - nothing to do" << endl;
        return 1;
    }
    
    bool debug = args->isSet("d");

    QString device = args->getOption("p");

    QextSerialPort* port = new QextSerialPort(device, QextSerialPort::Polling);
	port->setBaudRate(BAUD19200);
	port->setFlowControl(FLOW_XONXOFF);
	port->setParity(PAR_NONE);
	port->setDataBits(DATA_8);
	port->setStopBits(STOP_2);
	port->setTimeout(500);

    if (!port->open(QIODevice::ReadWrite))
    {
        cout << "Error: Cannot open port '" << device << "': " << strerror(errno) << endl;
        return 1;
    }

    QTime t;
    t.start();
    bool gotPrompt = false;
    while (!gotPrompt && (t.elapsed() < InitialTimeOut))
    {
        // "After a reset the bootloader waits for approximately 2 seconds to detect a
        //  transmission at its RXD pin. If so, it will measure the timing of the rising
        //  and falling edges of four consecutive characters 'U' at the host's baud to
        //  determine its correct baud rate prescaler."

        int written = port->write("UUUU", 4);

        if (debug)
            cout << "Written: " << written << endl;

        QString msg = port->read(100);

        if (debug)
            cout << "Read: " << msg << endl;

        int promptPos = msg.indexOf("c45b2");
        if (promptPos >= 0)
        {
            gotPrompt = true;
            QString blVersion = msg.mid(promptPos+6);
            int nlPos = blVersion.indexOf("\n");
            if (nlPos > 0)
                blVersion = blVersion.left(nlPos);
            cout << "Bootloader version " << blVersion << endl;
        }

        usleep(100000);
    }

    if (!gotPrompt)
    {
        cout << "Error: No initial reply from bootloader" << endl;
        return 1;
    }

    if (args->isSet("f"))
    {
        cout << "Programming flash memory.." << flush;
        port->write("pf\n", 3);
        QTime t;
        t.start();
        bool gotReply = false;
        while (!gotReply && (t.elapsed() < 100))
        {
            QString reply = port->read(10);
            if (reply.contains("pf+"))
            {
                gotReply = true;
                cout << "." << flush;
            }
        }
        if (!gotReply)
        {
            cout << "Error: Bootloader did not respond to 'pf' command" << endl;
            return 1;
        }
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
