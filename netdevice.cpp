#include "netdevice.h"
#include "udpClient.h"

NetDevice::NetDevice(QObject *parent) : QObject(parent)
{
    _ipAddress = new QHostAddress();
    _dataBuffer = new QByteArray();
}

bool NetDevice::setClient(QString address,QString port)
{
    _ipAddress->setAddress(address);
    _port = static_cast<quint16>(port.toUInt());

    tcpclient = new TcpClient();
    tcpclient->startConnect(address, _port);

    udpclient = new udpClient(_port);
    udpclient->start();

    return true;
}

qint32 NetDevice::readDevice(QByteArray &bufferIn)
{
    //tcpclient->getReadData(&bufferIn);

    return bufferIn.size();
}

qint32 NetDevice::writeDevice(quint8 *msgBuffer, quint16 msgSize)
{
    tcpclient->write(msgBuffer, msgSize);

    return 0;
}

void NetDevice::closeClient()
{
    //if(tcpclient->isRunning())
    //{
    //    tcpclient->quit();
    //    tcpclient->disableReconn();
    //}


    if(udpclient->isRunning())
    {
        udpclient->quit();
        udpclient->closeClient();
    }
}

void NetDevice::transmitDataReady()
{
    emit dataRead();
}
