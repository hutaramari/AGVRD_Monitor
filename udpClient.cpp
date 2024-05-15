#include "udpClient.h"
#include <QDebug>


udpClient::udpClient(quint16 p, QObject *parent) : QThread (parent)
{
    port = p;

    totalFrame = 0;
    lostFrame = 0;
    lastFrame = 0;
    isFirstTime_b = true;
    udpStartLoop = true;
    udpSocket = new QUdpSocket();
    datagram.clear();
}

udpClient::~udpClient()
{
    if(udpSocket != nullptr)
    {
        udpSocket->close();
        udpSocket = nullptr;
    }
}
void udpClient::closeClient()
{
//    if(udpSocket->hasPendingDatagrams())
//    {
//        udpSocket->receiveDatagram();
//    }
//    udpSocket->close();
    udpStartLoop = false;

}
void udpClient::run()
{
    qint64 ret;
    bool result;
    //udpSocket = new QUdpSocket();
    result = udpSocket->bind(QHostAddress::Any, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
//    result = udpSocket->bind(QHostAddress::Any, port, QUdpSocket::ShareAddress);
    qDebug()<<"\nudpClient::run\n";

    while(!result)
    {
        qDebug()<<"NG at "<<port;
        QThread::msleep(100);
        result = udpSocket->bind(QHostAddress::Any, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    }

    while(udpStartLoop)
    {
        if(udpSocket->hasPendingDatagrams())
        {
            datagram.resize(static_cast<int>(udpSocket->pendingDatagramSize()));
            QHostAddress sender;
            quint16 senderPort;

            ret = udpSocket->readDatagram(datagram.data(), datagram.size(),&sender, &senderPort);
            if(ret > 0)
            {
                if((_msgSize = this->protocol->decode(datagram, _msgBuffer, AGV_MODE_MDI)) > 0)
                {
                    dispatchMessage(_msgBuffer, _msgSize);
                }
            }
        }
        else
        {
            QThread::usleep(100);
        }
    }

    udpSocket->close();
}

qint32 udpClient::readMdi(QByteArray &bufferIn)
{
    bufferIn.append(datagram.data());

    return bufferIn.size();
}

void udpClient::setProtocol(Protocol *p)
{
    this->protocol = p;
}


void udpClient::dispatchMessage(quint8 *msgBuffer, quint32 bufSize)
{
    quint16 offset;
    quint16 idx;
    quint32 tmp;
    quint8  newRotation_b = 0;
    static quint8 packetIndex_b = 0;

    if(bufSize > PROTOCOL_BEA_MDIHEADERSIZE)
    {
        offset = 0;
        udpMdiFrame_s.header_s.pktType_b = msgBuffer[offset++];
        udpMdiFrame_s.header_s.pktSize_w = msgBuffer[offset++] * 256;
        udpMdiFrame_s.header_s.pktSize_w += msgBuffer[offset++];
        offset += 6;
        udpMdiFrame_s.header_s.pktNbr_w = msgBuffer[offset++] * 256;
        udpMdiFrame_s.header_s.pktNbr_w += msgBuffer[offset++];
        udpMdiFrame_s.header_s.totalNbr_b = msgBuffer[offset++];
        udpMdiFrame_s.header_s.subNbr_b = msgBuffer[offset++];
        udpMdiFrame_s.header_s.scanFreq_w = msgBuffer[offset++] * 256;
        udpMdiFrame_s.header_s.scanFreq_w += msgBuffer[offset++];
        udpMdiFrame_s.header_s.scanSpotsNbr_w = msgBuffer[offset++] * 256;
        udpMdiFrame_s.header_s.scanSpotsNbr_w += msgBuffer[offset++];
        // tmp = msgBuffer[offset++] * 256;
        // tmp += msgBuffer[offset++];
        // udpMdiFrame_s.header_s.firstAngle_sw = static_cast<qint16>(tmp);
        tmp  = msgBuffer[offset++] * 256 * 256 * 256;
        tmp += msgBuffer[offset++] * 256 * 256;
        tmp += msgBuffer[offset++] * 256;
        tmp += msgBuffer[offset++];
        udpMdiFrame_s.header_s.firstAngle_sdw = static_cast<qint32>(tmp);

        // tmp = msgBuffer[offset++] * 256;
        // tmp += msgBuffer[offset++];
        // udpMdiFrame_s.header_s.deltaAngle_sw = static_cast<qint16>(tmp);
        tmp  = msgBuffer[offset++] * 256 * 256 * 256;
        tmp += msgBuffer[offset++] * 256 * 256;
        tmp += msgBuffer[offset++] * 256;
        tmp += msgBuffer[offset++];
        udpMdiFrame_s.header_s.deltaAngle_sdw = static_cast<qint32>(tmp);

        udpMdiFrame_s.header_s.timeStamp_w = msgBuffer[offset++] * 256;
        udpMdiFrame_s.header_s.timeStamp_w += msgBuffer[offset++];

        // Combine MDI
        if(udpMdiFrame_s.header_s.scanSpotsNbr_w > 0)
        {
            // Find the 1st packet
            if(udpMdiFrame_s.header_s.subNbr_b == 1)
            {
                // Save first angle for MDI frame
                // udpMdiFrame_s.frame_s.startAngle_sw = udpMdiFrame_s.header_s.firstAngle_sw;
                udpMdiFrame_s.frame_s.startAngle_sdw = udpMdiFrame_s.header_s.firstAngle_sdw;

                // Clear spots number(because not copy to Dst buffer yet)
                udpMdiFrame_s.frame_s.spotNbr_w = 0;
                packetIndex_b++;
            }
            if(packetIndex_b > 0 && packetIndex_b == udpMdiFrame_s.header_s.subNbr_b)
            {
                // Valid start ==>
                // Calculate start address index for Dst buffer
                idx = udpMdiFrame_s.frame_s.spotNbr_w;
                for (quint16 i = 0; i < udpMdiFrame_s.header_s.scanSpotsNbr_w;i++)
                {
                    udpMdiFrame_s.distance_wa[idx + i] = msgBuffer[offset]*256+msgBuffer[offset+1];
                    offset += 2;
                }
                if(udpMdiFrame_s.header_s.pktType_b == 1)
                {
                    for (quint16 i = 0; i < udpMdiFrame_s.header_s.scanSpotsNbr_w;i++)
                    {
                        udpMdiFrame_s.pulsewidth_wa[idx + i] = msgBuffer[offset]*256+msgBuffer[offset+1];
                        offset += 2;
                    }
                }
                // Update spots number
                udpMdiFrame_s.frame_s.spotNbr_w += udpMdiFrame_s.header_s.scanSpotsNbr_w;

                // Update frame angle info
                // udpMdiFrame_s.frame_s.deltaAngle_sw = udpMdiFrame_s.header_s.deltaAngle_sw;
                udpMdiFrame_s.frame_s.deltaAngle_sdw = udpMdiFrame_s.header_s.deltaAngle_sdw;

                if(packetIndex_b == udpMdiFrame_s.header_s.totalNbr_b)
                {
                    newRotation_b = 1;
                    packetIndex_b = 0;
                }
                else {
                    packetIndex_b++;
                }
            }
            else
            {
                // Invalid start. Abandon packet until the 1st packet received
                qDebug()<<"UDP: Not beginning packet received!";
            }
        }

        // Increase total frame number
        totalFrame++;
        // Check frame sequence
        if(isFirstTime_b)
        {
            // First time, not check
            isFirstTime_b = false;
        }
        else
        {
            if(!(((udpMdiFrame_s.header_s.pktNbr_w - lastFrame) == 1) ||
                    (udpMdiFrame_s.header_s.pktNbr_w == 0 && lastFrame == 0xFFFF)))
            {
                lostFrame++;
            }
        }
        // Update last frame number
        lastFrame = udpMdiFrame_s.header_s.pktNbr_w;

        // emit signal
        if(newRotation_b == 1)
        {
            emit signalNewFrame();
        }
    }
}
