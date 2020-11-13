# Copyright 2011 Torsten Martinsen <bullestock@bullestock.net>

# This file is part of c45b.

# c45b is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# c45b is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with c45b.  If not, see <http://www.gnu.org/licenses/>.

include(../prefix.pri)

INCLUDEPATH += ../common ../ezOptionParser-0.0.0
LIBS += -lQt5SerialPort

TARGET = c45b

DEPENDPATH += ../ezOptionParser-0.0.0

CONFIG += console no_lflags_merge

c45b.path = $${EXEC_DIR}                                                                                                                                                                            

c45b.files = c45b
INSTALLS += c45b

HEADERS       = ../common/c45butils.h \
		../common/hexfile.h \
		../common/hexfiletester.h \
		../common/hexutils.h \
       		../common/platform.h \
       		../common/serport.h \
                ../ezOptionParser-0.0.0/ezOptionParser.hpp
SOURCES       = ../common/c45butils.cpp \
		../common/hexfile.cpp \
		../common/hexfiletester.cpp \
		../common/hexutils.cpp \
		../common/platform.cpp \
		../common/serport.cpp \
		main.cpp
