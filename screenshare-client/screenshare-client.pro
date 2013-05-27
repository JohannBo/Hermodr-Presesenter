#-------------------------------------------------
#
# Project created by QtCreator 2013-05-23T15:30:34
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = screenshare-client
TEMPLATE = app

INCLUDEPATH += ../QtWebsocket
DEPENDPATH += ../QtWebsocket

SOURCES += main.cpp\
    mainwindow.cpp \
    dialoglogin.cpp

HEADERS  += \
    mainwindow.h \
    dialoglogin.h \
    QWsSocket.h\
    QWsServer.h

FORMS    += \
    mainwindow.ui \
    dialoglogin.ui

win32:CONFIG(release, debug|release): LIBS += -L../QtWebsocket/release/ -lQtWebsocket
else:win32:CONFIG(debug, debug|release): LIBS += -L../QtWebsocket/debug/ -lQtWebsocket
else:unix: LIBS += -L../QtWebsocket/ -lQtWebsocket

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += ../QtWebsocket/release/QtWebsocket.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += ../QtWebsocket/debug/QtWebsocket.lib
else:unix: PRE_TARGETDEPS += ../QtWebsocket/libQtWebsocket.a
