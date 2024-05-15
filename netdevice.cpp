#include "netdevice.h"

NetDevice::NetDevice(QObject *parent) : QObject(parent)
{

}

bool NetDevice::setClient(QString address,QString port)
{
    _port = static_cast<quint16>(port.toUInt());

    tcpclient = new TcpClient(address, _port);
    tcpclient->start();
    tcpclient->setConnStart();

    udpclient = new udpClient(_port);
    udpclient->start();

    return true;
}

qint32 NetDevice::readDevice(QByteArray &bufferIn)
{
    qint32 size;
    QByteArray data;

    size = tcpclient->read(bufferIn);

    return size;
}

qint32 NetDevice::writeDevice(quint8 *msgBuffer, quint16 msgSize)
{
    tcpclient->write(msgBuffer, msgSize);

    return 0;
}

void NetDevice::closeClient()
{
    tcpclient->closeClient();
    tcpclient=nullptr;

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
