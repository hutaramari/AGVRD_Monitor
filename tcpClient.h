#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMutex>
#include <QByteArray>
#include <QTimer>
#include <QtNetwork>
#include "protocol.h"

class TcpClient : public QThread
{
    Q_OBJECT

public:
    explicit TcpClient(QString strAddressIP, quint16 port, QObject *parent = nullptr);
    ~TcpClient();
    void setProtocol(Protocol *p);
    void closeClient();
    void write(quint8 *msgBuffer, quint16 size);
    qint32 read(QByteArray &bufferIn);
    void setConnStart(void);

    MDI_Frame_t tcpMdiFrame_s;
    quint64 totalFrame;
    quint64 lostFrame;
    QTcpSocket *tcpSocket;
    bool tcpStartLoop;
    quint8 tcpCmdFrame_ba[100];
    quint32 tcpCmdLength_dw;

signals:
    void sigReadMdiData();
    void sigReadCmdData();

protected:
    void run();

private:
    QString ip_str;
    quint16 port_w;
    QTimer *m_timerConnect;
    bool isConnected;

    QByteArray datagram;
    Protocol *protocol;
    bool isFirstTime_b;
    quint8 tcpMsgBuffer[10*1024];
    quint8 tcpCmdBuffer[128];
    quint8 curFrameTotalNbr_b;

    void dispatchMessage(quint8 *msgBuffer, quint32 bufSize, quint8 myType);
    void onConnected(void);
    void onDisconnected(void);
};


#endif //TCPCLIENT_H
