#ifndef DEVICEHOST_H
#define DEVICEHOST_H

#include <QObject>
#include <QThread>

#include "serialdevice.h"
#include "netdevice.h"
#include "protocol.h"
#include "udpClient.h"

enum DeviceType{SerialPortDevice=1, NetPortDevice};

typedef struct{
    quint16 startAngle1;
    quint16 stopAngle1;
    quint16 spotsNbr1;

    quint16 startAngle2;
    quint16 stopAngle2;
    quint16 spotsNbr2;

    quint16 startAngle3;
    quint16 stopAngle3;
    quint16 spotsNbr3;

    quint16 startAngle4;
    quint16 stopAngle4;
    quint16 spotsNbr4;

    quint8  CTN;
    quint8  MDI;
    quint8  frameCounter;
    quint8  CAN;
    quint8  facetNumber;
    quint8  baudRate;
    quint8  heartBeat;

}uscanParameter_t;
Q_DECLARE_METATYPE(uscanParameter_t);

typedef struct{
    quint16 rpuVoltageErr;
    quint16 rpuVoltageErrCounter;
    quint32 rpuVoltageErrTime;

    quint16 motorErr;
    quint16 motorErrCounter;
    quint32 motorErrTime;

    quint16 lsrErr;
    quint16 lsrErrCounter;
    quint32 lsrErrTime;

    quint16 tdcVoltageErr;
    quint16 tdcVoltageErrCounter;
    quint32 tdcVoltageErrTime;

    quint16 tdcSeqErr;
    quint16 tdcSeqErrCounter;
    quint32 tdcSeqErrTime;

    quint16 ntcErr;
    quint16 ntcErrCounter;
    quint32 ntcErrTime;

    quint16 asyncErr;
    quint16 asyncErrCounter;
    quint16 asyncErrTime;

    quint32 softwareNbr;

    quint32 workingHour;

}DeviceLog_t;

typedef struct
{
    QString cmd;
    void (*decodeCmd)(quint8 bufferIn[], quint16 size);
}DeviceCmdTable_t;

#define WMS_LED_NUM (32)
#define WMS_LED_PULSE (17)
#define WMS_DATA_SIZE (WMS_LED_NUM * WMS_LED_PULSE)

class DeviceHost : public QObject
{
    Q_OBJECT
public:
    explicit DeviceHost(QObject *parent = nullptr);

    void setSerialHost(SerialDevice *device, Protocol *protocol);
    void closeSerialHost();

    void setNetHost(NetDevice *device, Protocol *protocol);
    void closeNetHost();

    void readLog(void);

    uscanParameter_t deviceParameter;

    DeviceLog_t deviceLog;

    bool parameterUpdated;

    quint64 totalFrame;
    quint64 lostFrame;

    MDI_Frame_t MDIFrame_s;
    qint16  temperature;

    quint16 wmsData[WMS_DATA_SIZE];

signals:
    void newMdiFrame();
    void newLog();
    void newParameter();
    void newWmsSignal(quint16 *buf_, quint16 size_);
    void temperatureSignal(void);

public slots:

    void stopDevice();
    void updateParameter();
    void storeParameter();

    void startAGV();
    void stopAGV();
    void updateTemperature();
    void getWmsSlot();

private:

    SerialDevice *serialDevice;
    NetDevice *netDevice;
    Protocol *protocol;

    //serial port
    void readSerialPortSlot(void);

    //ethernet port
    void readTcpMdiSlot(void);
    void readUdpMdiSlot(void);
    void readTcpCmdSlot(void);

    void readParameter(void);
    void sendCommand(QString cmd);

    QByteArray _bufferInArray;
    quint32 _msgSize;
    quint8 _msgBuffer[100];

    quint8 _bufferOut[READBUFSIZE];
    quint8 _bufferEncoded[READBUFSIZE];

    DeviceType deviceUsed;

    QStringList cmdList;

};

#endif // DEVICEHOST_H
