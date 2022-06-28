#include "serialdevice.h"

SerialDevice::SerialDevice(QObject *parent) : QObject(parent)
{
    serialPort = new QSerialPort();
    connect(serialPort, &QSerialPort::readyRead, this, &SerialDevice::transmitDataReady);

}

SerialDevice::~SerialDevice()
{
    serialPort->close();
    serialPort->deleteLater();
}

bool SerialDevice::setDevice(QString deviceName, QString deviceSpeed)
{

    bool result;

    if(QSerialPortInfo(deviceName).isBusy())
    {
        qDebug()<<"SerialDevice:setDevice return false";
        return false;
    }
    else
    {
        serialPort->setPortName(deviceName);
        serialPort->setBaudRate(deviceSpeed.toInt());
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
        serialPort->setReadBufferSize(READBUFSIZE);

        result = serialPort->open(QSerialPort::ReadWrite);

        qDebug()<<"SerialDevice:setDevice return "<<result;
    }

    return result;

}

void SerialDevice::closeDevice()
{
    serialPort->close();
    qDebug()<<"Close the SerialPort.";
}

void SerialDevice::transmitDataReady(void)
{
    emit dataRead();
}

qint32 SerialDevice::readDevice(QByteArray &bufferIn)
{
    if(!serialPort->atEnd())
    {
        bufferIn.append(serialPort->readAll());
    }

    return bufferIn.size();
}

qint32 SerialDevice::writeDevice(quint8 *msgBuffer, quint16 msgSize)
{
    qint64 result;

    result = serialPort->write(reinterpret_cast<char*>(msgBuffer), msgSize);

    return static_cast<quint32>(result);
}
