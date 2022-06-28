#ifndef SERIALDEVICE_H
#define SERIALDEVICE_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QThread>
#include <QDebug>

#define READBUFSIZE (5*1024)

class SerialDevice : public QObject
{
    Q_OBJECT
public:
    explicit SerialDevice(QObject *parent = nullptr);
    ~SerialDevice();

    bool setDevice(QString deviceName, QString deviceSpeed);

    qint32 readDevice(QByteArray &bufferIn);
    qint32 writeDevice(quint8 msgBuffer[], quint16 msgSize);

    void closeDevice();

signals:
    void dataRead(void);

private:
    QSerialPort *serialPort;

    QByteArray _tempArray;

    void transmitDataReady(void);

    QThread *serialDeviceThead;

};

#endif // SERIALDEVICE_H
