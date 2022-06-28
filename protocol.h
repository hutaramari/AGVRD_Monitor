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
const char PROTOCOL_version = 0x02;
const char PROTOCOL_BEA_HEADERSIZE = 11;
const char PROTOCOL_BEA_FOOTERSIZE = 2;


class Protocol : public QObject
{
    Q_OBJECT
public:
    explicit Protocol(QObject *parent = nullptr);

    void packCMD(quint16 cmd, quint8 buffer[], quint16 *cmdSize);
    void packCMD(QString cmd, quint8 buffer[], quint16 *cmdSize);
    quint32 decode(QByteArray &bufferInArray, quint8 bufferDecoded[]);
    quint32 decode(QByteArray &bufferInArray, quint8 bufferDecoded[], quint8 agvMode);
    quint16 encode(quint8 buffer[], quint8 bufferEncoded[], quint16 bufSize);
    quint16 encode(quint8 buffer[], quint8 bufferEncoded[], quint16 bufSize, quint8 agvMode);
    quint16 beaComputeCRC(quint8 msgBuffer[], quint16 msgSize);
    quint16 beaComputeCHK(quint8 msgBuffer[], quint16 msgSize);

    quint8 lobyte(quint16 w);
    quint8 hibyte(quint16 w);

signals:

public slots:
};

#endif // PROTOCOL_H
