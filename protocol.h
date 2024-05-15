#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>
#include <QDebug>

#define BEA_POLYNOM (0x90d9)

#define AGV_MODE_CMD 1
#define AGV_MODE_MDI 2

const char PROTOCOL_BEA_HEADER_0 = 0xBE;
const char PROTOCOL_BEA_HEADER_1 = 0xA0;
const char PROTOCOL_BEA_HEADER_2 = 0x12;
const char PROTOCOL_BEA_HEADER_3 = 0x34;
const char PROTOCOL_BEA_VERSION = 0x02;
const char PROTOCOL_BEA_HEADERSIZE = 11;
const char PROTOCOL_BEA_FOOTERSIZE = 2;
const char PROTOCOL_BEA_MDIHEADERSIZE = 31;
const char PROTOCOL_MDI_PKTTYPE_POS = 4;
const char PROTOCOL_MDI_PKTSIZE_POS = 5;
const char PROTOCOL_MDI_SCANPOINTS_POS = 19;

const char PROTOCOL_LEUZE_HEADER_0 = 0x4C;
const char PROTOCOL_LEUZE_HEADER_1 = 0x45;
const char PROTOCOL_LEUZE_HEADER_2 = 0x55;
const char PROTOCOL_LEUZE_HEADER_3 = 0x5A;
const char PROTOCOL_LEUZE_HEADER_4 = 0x45;
const char PROTOCOL_LEUZE_VERSION  = 0x01;
const char PROTOCOL_LEUZE_HEADERSIZE = 11;
const char PROTOCOL_LEUZE_FOOTERSIZE = 2;
const char PROTOCOL_LEUZE_MDIHEADERSIZE = 32;

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
    // qint16   firstAngle_sw;
    // qint16   deltaAngle_sw;
    qint32   firstAngle_sdw;
    qint32   deltaAngle_sdw;
    quint16  timeStamp_w;
}MDIHeader_t;
typedef struct
{
    // qint16  startAngle_sw;
    // qint16  deltaAngle_sw;
    qint32  startAngle_sdw;
    qint32  deltaAngle_sdw;
    quint16 spotNbr_w;
}AgvParameter_t;
typedef struct
{
    MDIHeader_t header_s;

    AgvParameter_t frame_s;

    quint16 distance_wa[1377*8];    // Max 1377spots * 2rotations in 40Hz
    quint16 pulsewidth_wa[1377*8];

}MDI_Frame_t;

class Protocol : public QObject
{
    Q_OBJECT
public:
    explicit Protocol(QObject *parent = nullptr);

    void packCMD(quint16 cmd, quint8 buffer[], quint16 *cmdSize);
    void packCMD(QString cmd, quint8 buffer[], quint16 *cmdSize);
    quint32 decode(QByteArray &bufferInArray, quint8 bufferDecoded[], quint8 cmdDecoded[]);
    quint32 decode(QByteArray &bufferInArray, quint8 bufferDecoded[], quint8 agvMode);
    quint16 encode(quint8 buffer[], quint8 bufferEncoded[], quint16 bufSize);
    quint16 encode(quint8 buffer[], quint8 bufferEncoded[], quint16 bufSize, quint8 agvMode);
    quint16 beaComputeCRC(quint8 msgBuffer[], quint16 msgSize);
    quint8 beaComputeCHK(quint8 msgBuffer[], quint16 msgSize);

    quint8 lobyte(quint16 w);
    quint8 hibyte(quint16 w);

    quint8 packetType;
    enum ProtocolType{PROTOCOL_BEA, PROTOCOL_LEUZE};
    void setMode(ProtocolType mode);

signals:

public slots:

private:
    ProtocolType mode_ = PROTOCOL_BEA;
};

#endif // PROTOCOL_H
