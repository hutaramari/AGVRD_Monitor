#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QThread>
#include <QtNetwork>
#include "protocol.h"

class udpClient : public QThread
{
    Q_OBJECT
public:
    explicit udpClient(quint16 p, QObject *parent = nullptr);
    ~udpClient();
    qint32 readMdi(QByteArray &bufferIn);
    void setProtocol(Protocol *p);
    void closeClient();

    MDI_Frame_t udpMdiFrame_s;
    quint64 totalFrame;
    quint64 lostFrame;
    quint16 lastFrame;
    QUdpSocket *udpSocket;
    bool udpStartLoop;

signals:
    void signalNewFrame();

protected:
    void run();

private:
    quint16 port;

    QByteArray datagram;
    Protocol *protocol;
    bool isFirstTime_b;
    quint32 _msgSize;
    quint8 _msgBuffer[10*1024];

    void dispatchMessage(quint8 *msgBuffer, quint32 bufSize);


private slots:

};

#endif // UDPCLIENT_H
