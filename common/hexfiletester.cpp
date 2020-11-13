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

#include "hexfile.h"
#include "hexfiletester.h"
#include "hexutils.h"

using namespace std;

void HexFileTester::test(const QString& filename)
{
    HexFile hf;

    // test load
    if(!hf.load(filename, true))
    {
        cout << "Error opening hexfile '" << filename.toLatin1().constData() << "': " << hf.errorString().toLatin1().constData() << endl;
        return;
    }

    // test write routine
    if (!writeHexfile(filename+"_out2.hex", hf))
        return;

    // test empty file
    hf.reset();
    if (!writeHexfile(filename+"_out3.hex", hf))
        return;

    // test add
    hf.reset();
    hf.append('h');
    hf.append('a');
    hf.append('l');
    hf.append('l');
    hf.append('0');
    hf.append(0);
    hf.append(1);
    hf.append(2);
    hf.append(3);
    hf.append(4);
    if (!writeHexfile(filename+"_out4.hex", hf))
        return;

    // test set
    hf.setByte(1, 'e');
    hf.setByte(8, 255);
    if (!writeHexfile(filename+"_out5.hex", hf))
        return;

    // test set after current end
    hf.setByte(654, 255);
    hf.append(0);
    hf.append(1);
    hf.append(2);
    hf.append(3);
    hf.append(4);
    if (!writeHexfile(filename+"_out6.hex", hf))
        return;

    // test extended (>64k) range
    hf.setByte(0x10000, 0x11);
    hf.setByte(0x105FF, 0x22);
    hf.append(0);
    hf.append(1);
    hf.append(2);
    hf.append(3);
    hf.append(4);
    hf.setByte(0x205FF, 0x33);
    if (!writeHexfile(filename+"_out7.hex", hf))
        return; // TODO: Complain?
}
