#ifndef UDPCLIENT_H
#define UDPCLIENT_H

#include <QThread>
#include <QtNetwork>
#include "protocol.h"

// For AGV RD sensor
typedef struct
{
    quint8   pktType_b;
    quint16  pktSize_w;
    quint16  reserved01_w;
    quint16  reserved02_w;
    quint16  reserved03_w;
    quint16  pktNbr_w;
    quint8   totalNbr_b;
    quint8   subNbr_b;
    quint16  scanFreq_w;
    quint16  scanSpotsNbr_w;
    qint16   firstAngle_sw;
    qint16   deltaAngle_sw;
    quint16  timeStamp_w;
}MDIHeader_t;
// For AGV RD sensor
typedef struct
{
    qint16  startAngle_sw;
    qint16  stopAngle_sw;
    qint16  deltaAngle_sw;
    quint16 spotNbr_w;
}AgvParameter_t;
typedef struct
{
    MDIHeader_t header_s;

    AgvParameter_t frame_s;

    quint16 distance_wa[1377*2];    // Max 1377spots * 2rotations in 40Hz
    quint16 pulsewidth_wa[1377*2];

}MDI_Frame_t;

class udpClient : public QThread
{
    Q_OBJECT
public:
    explicit udpClient(quint16 p, QObject *parent = nullptr);
    ~udpClient();
    qint32 readMdi(QByteArray &bufferIn);
    void setProtocol(Protocol *p);
    void closeClient();

    MDI_Frame_t mdiFrame_s;
    quint64 totalFrame;
    quint64 lostFrame;
    quint16 lastFrame;
    QUdpSocket *mdiSocket;
    bool startLoop;

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
