#ifndef ETHERNET_H
#define ETHERNET_H

#include <QObject>
#include <QtNetwork>
#include <QHostAddress>

#include "udpClient.h"
#include "tcpClient.h"

#define WAIT_TIME 2000

class NetDevice : public QObject
{
    Q_OBJECT
public:
    explicit NetDevice(QObject *parent = nullptr);

    bool setClient(QString address, QString port);

    qint32 readDevice(QByteArray &bufferIn);
    qint32 writeDevice(quint8 msgBuffer[], quint16 msgSize);

    void closeClient();

    udpClient *udpclient;
    TcpClient *tcpclient;

signals:
    void dataRead(void);

private:
    quint16 _port;

    void transmitDataReady(void);
};

#endif // ETHERNET_H
