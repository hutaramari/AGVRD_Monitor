#-------------------------------------------------
#
# Project created by QtCreator 2019-04-04T13:55:57
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
QT       += charts
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = AGV_RD_Monitor
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11
CONFIG += resources_big

SOURCES += \
    alg.cpp \
    chartview.cpp \
    config.cpp \
        main.cpp \
        mainwindow.cpp \
    qcustomplot.cpp \
    serialdevice.cpp \
    protocol.cpp \
    devicehost.cpp \
    netdevice.cpp \
    tcpClient.cpp \
    udpClient.cpp \
    wmsdialog.cpp

HEADERS += \
    alg.h \
    chartview.h \
    config.h \
        mainwindow.h \
    qcustomplot.h \
    serialdevice.h \
    protocol.h \
    devicehost.h \
    netdevice.h \
    tcpClient.h \
    udpClient.h \
    wmsdialog.h

FORMS += \
    config.ui \
        mainwindow.ui \
    devicedialog.ui \
    aboutdialog.ui \
    parameterdialog.ui \
    wmsdialog.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc

RC_FILE = AGV_RD_Monitor_resource.rc
