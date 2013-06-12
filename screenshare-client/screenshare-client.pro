# Copyright (C) 2013
# johann.bornholdt@gmail.com
#
# This file is part of Hermodr-Presenter.
#
# Hermodr-Presenter is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Hermodr-Presenter is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Hermodr-Presenter.  If not, see <http://www.gnu.org/licenses/>.

#-------------------------------------------------
#
# Project created by QtCreator 2013-05-23T15:30:34
#
#-------------------------------------------------

QT       += core gui network multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = screenshare-client
TEMPLATE = app

INCLUDEPATH += ../QtWebsocket
DEPENDPATH += ../QtWebsocket

SOURCES += main.cpp\
    mainwindow.cpp

HEADERS  += \
    mainwindow.h \
    QWsSocket.h\
    QWsServer.h

FORMS    += \
    mainwindow.ui

win32:CONFIG(release, debug|release): LIBS += -L../QtWebsocket/release/ -lQtWebsocket
else:win32:CONFIG(debug, debug|release): LIBS += -L../QtWebsocket/debug/ -lQtWebsocket
else:unix: LIBS += -L../QtWebsocket/ -lQtWebsocket

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += ../QtWebsocket/release/QtWebsocket.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../QtWebsocket/debug/QtWebsocket.lib
else:unix: PRE_TARGETDEPS += ../QtWebsocket/libQtWebsocket.a
