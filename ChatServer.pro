#-------------------------------------------------
#
# Project created by QtCreator 2017-10-30T18:38:14
#
#-------------------------------------------------

QT       += core gui network sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChatServer
TEMPLATE = app


SOURCES += main.cpp\
        chatserver.cpp \
    oneclient.cpp

HEADERS  += chatserver.h \
    oneclient.h

FORMS    += chatserver.ui

RESOURCES += \
    resource.qrc
