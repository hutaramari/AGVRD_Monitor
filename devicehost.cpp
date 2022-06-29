#include "devicehost.h"


DeviceHost::DeviceHost(QObject *parent) : QObject(parent)
{
    parameterUpdated = false;

    totalFrame = 0;
    lostFrame = 0;

    // NOTE: If you changed item's order of cmdList,
    // please modify the index of process in dispatchMessage().
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

    while( (_msgSize = this->protocol->decode(_bufferInArray, _msgBuffer)) > 0)
    {
        dispatchMessage(_msgBuffer);
    }
}

void DeviceHost::dispatchMessage(quint8 msgBuffer[])
{
    /* dispatch message from serial port */
}

/*
 * Dispatch reponse from AGV sensor
*/
void DeviceHost::dispatchMessage(quint8 *msgBuffer, quint32 bufSize, quint8 agvMode)
{
    QString strCmd;
    QByteArray cmd;
    qint16 pos = -1;

    if(agvMode == AGV_MODE_CMD)
    {
        // Transfer data to string
        for (quint32 i = 0; i < bufSize; i++)
        {
            cmd.append(static_cast<char>(msgBuffer[i]));
        }
        strCmd = QString::fromUtf8(cmd);

        // Find command postion
        for(quint16 i = 0; i < cmdList.size(); i++)
        {
            if(strCmd.contains(cmdList.at(i)))
            {
                pos = static_cast<qint16>(i);
                break; // found and exit
            }
        }
        if(pos >= 0)
        {
            switch (pos)
            {
            case 0: // (!!!Follow the order of cmdList)
                qDebug()<<"Started receiving MDI";
                break;
            case 1:
                qDebug()<<"Stoped receiving MDI";
                break;
            case 18:
                if(bufSize == 13)
                {
                    quint16 tmp= msgBuffer[11] * 256 + msgBuffer[12];
                    temperature = static_cast<qint16>(tmp);
                    emit temperatureSignal();
                }
                break;
            case 20:
                qDebug()<<"WMS data received: "<<bufSize;
                if(bufSize == (WMS_DATA_SIZE*2 + 11))
                {
                    for(int i = 0; i < WMS_DATA_SIZE; i++)
                    {
                        wmsData[i] = msgBuffer[11+2*i]*256 + msgBuffer[11+2*i+1];
                    }
                    emit newWmsSignal(wmsData, WMS_DATA_SIZE);
                }
                break;
            default:
                break;
            }

            parameterUpdated = true;
//            emit newParameter();
        }
        else
        {
            qDebug()<<"Not find the command.";
        }
    }
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

    //connect(this->netDevice, &NetDevice::dataRead, this, &DeviceHost::readNetDeviceSlot);
    connect(this->netDevice->tcpclient, &TcpClient::signalReadData, this, &DeviceHost::readNetDeviceSlot);
    connect(this->netDevice->udpclient, &udpClient::signalNewFrame, this, &DeviceHost::readNetDeviceMdiSlot);
    this->netDevice->udpclient->setProtocol(protocol);
    deviceUsed = DeviceType::NetPortDevice;
}

void DeviceHost::closeNetHost()
{
    if(this->netDevice != nullptr)
    {
        //disconnect(this->netDevice, &NetDevice::dataRead, this, &DeviceHost::readNetDeviceSlot);
        disconnect(this->netDevice->tcpclient, &TcpClient::signalReadData, this, &DeviceHost::readNetDeviceSlot);
        disconnect(this->netDevice->udpclient, &udpClient::signalNewFrame, this, &DeviceHost::readNetDeviceMdiSlot);

        this->netDevice->closeClient();
        this->netDevice = nullptr;
    }
}

/*
 * Read and decode CMD frame from AGV sensor
*/
void DeviceHost::readNetDeviceSlot(void)
{
    this->netDevice->readDevice(_bufferInArray);

    while( (_msgSize = this->protocol->decode(_bufferInArray, _msgBuffer, AGV_MODE_CMD)) > 0)
    {
        dispatchMessage(_msgBuffer, _msgSize, AGV_MODE_CMD);
    }
}

void DeviceHost::readNetDeviceMdiSlot(void)
{
    memcpy(&MDIFrame_s.header_s.pktType_b, &this->netDevice->udpclient->mdiFrame_s.header_s.pktType_b, sizeof (MDI_Frame_t));
    totalFrame = this->netDevice->udpclient->totalFrame;
    lostFrame = this->netDevice->udpclient->lostFrame;
    lastFrame = this->netDevice->udpclient->lastFrame;
    emit newFrame();
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
