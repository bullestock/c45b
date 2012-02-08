// Copyright 2012 Torsten Martinsen <bullestock@bullestock.net>

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

#include <QFile>
#include <QString>

#include "hexfile.h"

using namespace std;

bool writeHexfile(const QString& filename, const HexFile& hf)
{
    QFile out(filename);
    if (!out.open(QIODevice::WriteOnly))
    {
        cout << "Could not write file '" << filename.data() << "'" << endl;
        return false;
    }
    out.write(hf.getHexFile().join("").toLocal8Bit());
    out.close();
    return true;
}


unsigned char asciiToHex(unsigned char a)
{
    if(a >= 'a')
        return a - 'a' + 10;

    if(a >= 'A')
        return a - 'A' + 10;
    
    if (a >= '0')
        return a - '0';

    return 0;
}

unsigned char asciiToHex(unsigned char high, unsigned char low)
{
    return (asciiToHex(high) << 4) + asciiToHex(low);
}
