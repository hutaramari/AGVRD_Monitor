#include "devicehost.h"


DeviceHost::DeviceHost(QObject *parent) : QObject(parent)
{
    parameterUpdated = false;

    totalFrame = 0;
    lostFrame = 0;

    cmdList
        <<QStringLiteral("cWA SendMDI") // 0
        <<QStringLiteral("cWA StopMDI") // 1
        <<QStringLiteral("cRA GetIP")   // 2
        <<QStringLiteral("cRA GetGW")   // 3
        <<QStringLiteral("cRA GetMask") // 4
        <<QStringLiteral("cRA GetDHCP") // 5
        <<QStringLiteral("cRA GetProto")// 6
        <<QStringLiteral("cRA GetPort") // 7
        <<QStringLiteral("cRA GetPType")// 8
        <<QStringLiteral("cRA GetResol")// 9
        <<QStringLiteral("cRA GetIncre")// 10
        <<QStringLiteral("cRA GetRange")// 11
        <<QStringLiteral("cRA GetSkip") // 12
        <<QStringLiteral("cRA GetImmu") // 13
        <<QStringLiteral("cRA GetFog")  // 14
        <<QStringLiteral("cRA GetCont") // 15
        <<QStringLiteral("cRA GetStat") // 16
        <<QStringLiteral("cRA GetVer")  // 17
        <<QStringLiteral("cRA GetTem")  // 18
        <<QStringLiteral("cRA GetELog") // 19
        <<QStringLiteral("cRA GetWms")  // 20
           ;
}

void DeviceHost::setSerialHost(SerialDevice *device, Protocol *protocol)
{
    this->serialDevice = device;
    this->protocol = protocol;

    connect(this->serialDevice, &SerialDevice::dataRead, this, &DeviceHost::readSerialPortSlot);

    qDebug()<<"DeviceHost: setserialHost";
    deviceUsed = DeviceType::SerialPortDevice;

    readParameter();

}

void DeviceHost::closeSerialHost()
{
    if(this->serialDevice != nullptr)
    {
        disconnect(this->serialDevice, &SerialDevice::dataRead, this, &DeviceHost::readSerialPortSlot);
        this->serialDevice->closeDevice();
        this->serialDevice = nullptr;
    }
}

void DeviceHost::readSerialPortSlot(void)
{
    /*
     * read raw data stream from serial port
     * decode and parse the message with protocol
     * then emit new data to ui
    */

    this->serialDevice->readDevice(_bufferInArray);
}

void DeviceHost::readParameter(void)
{
    quint16 cmdBufSize;
    quint16 msgBufSize;

    this->protocol->packCMD(50004, _bufferOut, &cmdBufSize);
    msgBufSize = this->protocol->encode(_bufferOut, _bufferEncoded, cmdBufSize);

    if(deviceUsed == DeviceType::SerialPortDevice)
    {
        this->serialDevice->writeDevice(_bufferEncoded, msgBufSize);
    }
    else
    {
        this->netDevice->writeDevice(_bufferEncoded, msgBufSize);
    }
}

/*
 * This function only support <GET> command for AGV sensor
*/
void DeviceHost::sendCommand(QString cmd)
{
    quint16 cmdBufSize;
    quint16 msgBufSize;

    this->protocol->packCMD(cmd, _bufferOut, &cmdBufSize);
    msgBufSize = this->protocol->encode(_bufferOut, _bufferEncoded, cmdBufSize, AGV_MODE_CMD);

    if(deviceUsed == DeviceType::NetPortDevice)
    {
        this->netDevice->writeDevice(_bufferEncoded, msgBufSize);
    }
}

void DeviceHost::readLog(void)
{
    quint16 cmdBufSize;
    quint16 msgBufSize;

    this->protocol->packCMD(49396, _bufferOut, &cmdBufSize);
    msgBufSize = this->protocol->encode(_bufferOut, _bufferEncoded, cmdBufSize);

    if(deviceUsed == DeviceType::SerialPortDevice)
    {
        this->serialDevice->writeDevice(_bufferEncoded, msgBufSize);
    }
    else
    {
        this->netDevice->writeDevice(_bufferEncoded, msgBufSize);
    }

}

void DeviceHost::setNetHost(NetDevice *device, Protocol *protocol)
{
    this->netDevice = device;
    this->protocol = protocol;

    connect(this->netDevice->tcpclient, &TcpClient::sigReadMdiData, this, &DeviceHost::readTcpMdiSlot);
    connect(this->netDevice->udpclient, &udpClient::signalNewFrame, this, &DeviceHost::readUdpMdiSlot);
    connect(this->netDevice->tcpclient, &TcpClient::sigReadCmdData, this, &DeviceHost::readTcpCmdSlot);
    this->netDevice->tcpclient->setProtocol(protocol);
    this->netDevice->udpclient->setProtocol(protocol);
    deviceUsed = DeviceType::NetPortDevice;
}

void DeviceHost::closeNetHost()
{
    if(this->netDevice != nullptr)
    {
        //disconnect(this->netDevice, &NetDevice::dataRead, this, &DeviceHost::readTcpMdiSlot);
        disconnect(this->netDevice->tcpclient, &TcpClient::sigReadMdiData, this, &DeviceHost::readTcpMdiSlot);
        disconnect(this->netDevice->udpclient, &udpClient::signalNewFrame, this, &DeviceHost::readUdpMdiSlot);
        disconnect(this->netDevice->tcpclient, &TcpClient::sigReadCmdData, this, &DeviceHost::readTcpCmdSlot);
        this->netDevice->closeClient();
        this->netDevice = nullptr;
    }
}

/*
 * Read and decode MDI frame from AGV sensor
*/
void DeviceHost::readTcpMdiSlot(void)
{
    memcpy(&MDIFrame_s.header_s.pktType_b, &this->netDevice->tcpclient->tcpMdiFrame_s.header_s.pktType_b, sizeof (MDI_Frame_t));
    totalFrame = this->netDevice->tcpclient->totalFrame;
    lostFrame = this->netDevice->tcpclient->lostFrame;
    emit newMdiFrame();
}

void DeviceHost::readUdpMdiSlot(void)
{
    memcpy(&MDIFrame_s.header_s.pktType_b, &this->netDevice->udpclient->udpMdiFrame_s.header_s.pktType_b, sizeof (MDI_Frame_t));
    totalFrame = this->netDevice->udpclient->totalFrame;
    lostFrame = this->netDevice->udpclient->lostFrame;
    emit newMdiFrame();
}

void DeviceHost::readTcpCmdSlot(void)
{
    QString cmd="";
    quint32 len;

    len = this->netDevice->tcpclient->tcpCmdLength_dw;
    if(len > sizeof(_msgBuffer))
    {
        len = sizeof(_msgBuffer);
    }
    memcpy(_msgBuffer, &this->netDevice->tcpclient->tcpCmdFrame_ba, len);

    for (quint32 i = 0;i < len; i++) {
        cmd.append(_msgBuffer[i]);
    }

    if(cmd.contains(cmdList[18]))
    {
        temperature = _msgBuffer[len - 2] * 256 + _msgBuffer[len - 1];
        emit temperatureSignal();
    }
}

void DeviceHost::stopDevice()
{

}

void DeviceHost::updateParameter()
{

}

void DeviceHost::storeParameter()
{

}

void DeviceHost::startAGV()
{
    sendCommand("cWN SendMDI");
}

void DeviceHost::stopAGV()
{
    sendCommand("cWN StopMDI");
}

void DeviceHost::updateTemperature()
{
    sendCommand("cRN GetTem");
}

void DeviceHost::getWmsSlot()
{
    sendCommand("cRN GetWms");
}
