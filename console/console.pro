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

INCLUDEPATH += ../qserialport/include ../common ../ezOptionParser-0.0.0
LIBS += -L../qserialport/lib -lQtSerialPort

TARGET = c45b

DEPENDPATH += ../qserialport/lib ../ezOptionParser-0.0.0

CONFIG += console no_lflags_merge

isEmpty(PREFIX) {
    PREFIX = /usr/local
}

win32: c45b.path = "$$(PROGRAMFILES_HOME)"

!win32: c45b.path = $$(PREFIX)/bin
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
