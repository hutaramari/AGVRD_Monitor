#include "udpClient.h"
#include <QDebug>


udpClient::udpClient(quint16 p, QObject *parent) : QThread (parent)
{
    port = p;

    totalFrame = 0;
    lostFrame = 0;
    lastFrame = 0;
    isFirstTime_b = true;
    startLoop = true;
}

udpClient::~udpClient()
{
    if(mdiSocket != nullptr)
    {
        mdiSocket->close();
        mdiSocket = nullptr;
    }
}
void udpClient::closeClient()
{
//    if(mdiSocket->hasPendingDatagrams())
//    {
//        mdiSocket->receiveDatagram();
//    }
//    mdiSocket->close();
    startLoop = false;

}
void udpClient::run()
{
    qint64 ret;
    bool result;
    mdiSocket = new QUdpSocket();
    result = mdiSocket->bind(QHostAddress::Any, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
//    result = mdiSocket->bind(QHostAddress::Any, port, QUdpSocket::ShareAddress);
    qDebug()<<"udpClient::run";

#if 1
    while(!result)
    {
        qDebug()<<"NG at "<<port;
        QThread::msleep(100);
        result = mdiSocket->bind(QHostAddress::Any, port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    }
#endif

    while(startLoop)
    {
        if(mdiSocket->hasPendingDatagrams())
        {
            datagram.resize(mdiSocket->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;

            ret = mdiSocket->readDatagram(datagram.data(), datagram.size(),&sender, &senderPort);
            if(ret>0)
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

    mdiSocket->close();
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
    quint16 tmp;
    quint8  newRotation_b = 0;
    static quint8 packetIndex_b = 0;

    if(bufSize > 27)
    {
        offset = 0;
        //TODO: complete mdiFrame_s
        mdiFrame_s.header_s.pktType_b = msgBuffer[offset++];
        mdiFrame_s.header_s.pktSize_w = msgBuffer[offset++] * 256;
        mdiFrame_s.header_s.pktSize_w += msgBuffer[offset++];
        offset += 6;
        mdiFrame_s.header_s.pktNbr_w = msgBuffer[offset++] * 256;
        mdiFrame_s.header_s.pktNbr_w += msgBuffer[offset++];
        mdiFrame_s.header_s.totalNbr_b = msgBuffer[offset++];
        mdiFrame_s.header_s.subNbr_b = msgBuffer[offset++];
        mdiFrame_s.header_s.scanFreq_w = msgBuffer[offset++] * 256;
        mdiFrame_s.header_s.scanFreq_w += msgBuffer[offset++];
        mdiFrame_s.header_s.scanSpotsNbr_w = msgBuffer[offset++] * 256;
        mdiFrame_s.header_s.scanSpotsNbr_w += msgBuffer[offset++];
        tmp = msgBuffer[offset++] * 256;
        tmp += msgBuffer[offset++];
        mdiFrame_s.header_s.firstAngle_sw = static_cast<qint16>(tmp);
        tmp = msgBuffer[offset++] * 256;
        tmp += msgBuffer[offset++];
//        mdiFrame_s.header_s.deltaAngle_sw = msgBuffer[offset++] * 256;
//        mdiFrame_s.header_s.deltaAngle_sw += msgBuffer[offset++];
        mdiFrame_s.header_s.deltaAngle_sw = static_cast<qint16>(tmp);
        mdiFrame_s.header_s.timeStamp_w = msgBuffer[offset++] * 256;
        mdiFrame_s.header_s.timeStamp_w += msgBuffer[offset++];

        // Combine MDI
        if(mdiFrame_s.header_s.scanSpotsNbr_w > 0)
        {
            // Find the 1st packet
            if(mdiFrame_s.header_s.subNbr_b == 1)
            {
                // Save first angle for MDI frame
                mdiFrame_s.frame_s.startAngle_sw = mdiFrame_s.header_s.firstAngle_sw;
                // Clear spots number(because not copy to Dst buffer yet)
                mdiFrame_s.frame_s.spotNbr_w = 0;
                packetIndex_b++;
            }
            if(packetIndex_b > 0 && packetIndex_b == mdiFrame_s.header_s.subNbr_b)
            {
                // Valid start ==>
                // Calculate start address index for Dst buffer
                idx = mdiFrame_s.frame_s.spotNbr_w;
                for (quint16 i = 0; i < mdiFrame_s.header_s.scanSpotsNbr_w;i++)
                {
                    mdiFrame_s.distance_wa[idx + i] = msgBuffer[offset]*256+msgBuffer[offset+1];
                    offset += 2;
                }
                if(mdiFrame_s.header_s.pktType_b == 1)
                {
                    for (quint16 i = 0; i < mdiFrame_s.header_s.scanSpotsNbr_w;i++)
                    {
                        mdiFrame_s.pulsewidth_wa[idx + i] = msgBuffer[offset]*256+msgBuffer[offset+1];
                        offset += 2;
                    }
                }
                // Update spots number
                mdiFrame_s.frame_s.spotNbr_w += mdiFrame_s.header_s.scanSpotsNbr_w;

                // Update frame angle info
                mdiFrame_s.frame_s.deltaAngle_sw = mdiFrame_s.header_s.deltaAngle_sw;
                mdiFrame_s.frame_s.stopAngle_sw = mdiFrame_s.header_s.firstAngle_sw + \
                        mdiFrame_s.header_s.deltaAngle_sw / 10 * (mdiFrame_s.header_s.scanSpotsNbr_w - 1);

                if(packetIndex_b == mdiFrame_s.header_s.totalNbr_b)
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
                qDebug()<<"Not beginning packet received!";
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
            if(!(((mdiFrame_s.header_s.pktNbr_w - lastFrame) == 1) ||
                    (mdiFrame_s.header_s.pktNbr_w == 0 && lastFrame == 0xFFFF)))
            {
                lostFrame++;
            }
        }
        // Update last frame number
        lastFrame = mdiFrame_s.header_s.pktNbr_w;

        // emit signal
        if(newRotation_b == 1)
        {
            emit signalNewFrame();
        }
    }
}
