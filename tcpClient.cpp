#include "tcpClient.h"

#include <QtWidgets>


TcpClient::TcpClient(QString strAddressIP, quint16 port, QObject *parent) : QThread(parent)
{
    lostFrame = 0;
    totalFrame = 0;
    isFirstTime_b = true;
    tcpStartLoop = true;
    ip_str = strAddressIP;
    port_w = port;

    isConnected = false;
    tcpSocket = new QTcpSocket();
    m_timerConnect = new QTimer();

    tcpCmdLength_dw = 0;
    curFrameTotalNbr_b = 0;
}

TcpClient::~TcpClient()
{
    if(tcpSocket != nullptr)
    {
        tcpSocket->abort();
        tcpSocket = nullptr;
    }
}

void TcpClient::write(quint8 *msgBuffer, quint16 size)
{
    if(tcpSocket->state() != QAbstractSocket::ConnectedState)
        return;

    qint64 iSendLen = tcpSocket->write(reinterpret_cast<char*>(msgBuffer), size);
    if(!tcpSocket->flush() || iSendLen < 0)
    {
        tcpSocket->disconnectFromHost();
        tcpSocket->waitForDisconnected(3000);
    }
}

void TcpClient::closeClient()
{
    qDebug()<<"Closed.";
    tcpStartLoop = false;
    m_timerConnect->stop();
    tcpSocket->close();
}

void TcpClient::run()
{
    qint64 ret;
    qint32 msg_index;
    quint32 size = 0;
    quint8 drop = 1;
    quint8 packetNum;

    //tcpSocket = new QTcpSocket();


    connect(this->tcpSocket, &QTcpSocket::connected,this, &TcpClient::onConnected);
    connect(this->tcpSocket, &QTcpSocket::disconnected,this, &TcpClient::onDisconnected);
    connect(m_timerConnect, &QTimer::timeout, [=](){
        if(isConnected)
            return;
        if(tcpSocket->state() != QAbstractSocket::UnconnectedState)
            return;
        tcpSocket->connectToHost(ip_str, port_w);
    });

    qDebug()<<"tcpClient::run";

    while(tcpStartLoop)
    {
        if(tcpSocket->state() != QAbstractSocket::ConnectedState)
        {
            qDebug()<<"tcpClient::wait to connected (error = " << tcpSocket->errorString() << " )";
            QThread::msleep(100);
            continue;
        }
        ret = tcpSocket->bytesAvailable();
        if(ret > 0)
        {
            //datagram.resize(static_cast<int>(ret));
            datagram.clear();
            try {
                datagram = tcpSocket->readAll();
            } catch (...) {
                qDebug()<< "TCP read error = "<<tcpSocket->errorString();
            }

            //qDebug()<< "TCP read: " << datagram.size();
            if(drop)
            {
                drop = 0;
            }
            else
            {
                msg_index = 0;
                packetNum = 0;
                do
                {
                    if((size = this->protocol->decode(datagram, &tcpMsgBuffer[msg_index], tcpCmdFrame_ba)) > 0)
                    {
                        dispatchMessage(&tcpMsgBuffer[msg_index], size, this->protocol->packetType);
                        if(this->protocol->packetType == AGV_MODE_CMD)
                        {
                            tcpCmdLength_dw = size;
                            size = 0;
                        }
                        else if(this->protocol->packetType == AGV_MODE_MDI)
                        {
                            if(datagram.size() >= static_cast<qint32>(size))
                            {
                                datagram.remove(0, static_cast<qint32>(size));
                            }
                            msg_index += size;
                            if(++packetNum == curFrameTotalNbr_b)
                            {
                                msg_index = 0;
                                packetNum = 0;
                            }
                        }
                    }
                } while((datagram.size() > 0) && (size > 0));
            }
        }
        else {
            QThread::msleep(1);
        }
    }
}

qint32 TcpClient::read(QByteArray &bufferIn)
{
    bufferIn.append(datagram.data());

    return bufferIn.size();
}

void TcpClient::setProtocol(Protocol *p)
{
    this->protocol = p;
}

void TcpClient::setConnStart(void)
{
    m_timerConnect->start(1000);
}

void TcpClient::dispatchMessage(quint8 *msgBuffer, quint32 bufSize, quint8 myType)
{
    quint16 offset;
    quint16 idx;
    quint32 tmp;
    quint8  newRotation_b = 0;

    if(myType == AGV_MODE_CMD)
    {
        emit sigReadCmdData();
    }
    else if(myType == AGV_MODE_MDI)
    {
        if(bufSize > PROTOCOL_BEA_MDIHEADERSIZE)
        {
            offset = 0;
            tcpMdiFrame_s.header_s.pktType_b = msgBuffer[offset++];
            tcpMdiFrame_s.header_s.pktSize_w = msgBuffer[offset++] * 256;
            tcpMdiFrame_s.header_s.pktSize_w += msgBuffer[offset++];
            offset += 6;
            tcpMdiFrame_s.header_s.pktNbr_w = msgBuffer[offset++] * 256;
            tcpMdiFrame_s.header_s.pktNbr_w += msgBuffer[offset++];
            tcpMdiFrame_s.header_s.totalNbr_b = msgBuffer[offset++];
            tcpMdiFrame_s.header_s.subNbr_b = msgBuffer[offset++];
            tcpMdiFrame_s.header_s.scanFreq_w = msgBuffer[offset++] * 256;
            tcpMdiFrame_s.header_s.scanFreq_w += msgBuffer[offset++];
            tcpMdiFrame_s.header_s.scanSpotsNbr_w = msgBuffer[offset++] * 256;
            tcpMdiFrame_s.header_s.scanSpotsNbr_w += msgBuffer[offset++];
            // tmp = msgBuffer[offset++] * 256;
            // tmp += msgBuffer[offset++];
            // tcpMdiFrame_s.header_s.firstAngle_sw = static_cast<qint16>(tmp);
            tmp  = msgBuffer[offset++] * 256 * 256 * 256;
            tmp += msgBuffer[offset++] * 256 * 256;
            tmp += msgBuffer[offset++] * 256;
            tmp += msgBuffer[offset++];
            tcpMdiFrame_s.header_s.firstAngle_sdw = static_cast<qint32>(tmp);

            // tmp = msgBuffer[offset++] * 256;
            // tmp += msgBuffer[offset++];
            // tcpMdiFrame_s.header_s.deltaAngle_sw = static_cast<qint16>(tmp);
            tmp  = msgBuffer[offset++] * 256 * 256 * 256;
            tmp += msgBuffer[offset++] * 256 * 256;
            tmp += msgBuffer[offset++] * 256;
            tmp += msgBuffer[offset++];
            tcpMdiFrame_s.header_s.deltaAngle_sdw = static_cast<qint32>(tmp);

            tcpMdiFrame_s.header_s.timeStamp_w = msgBuffer[offset++] * 256;
            tcpMdiFrame_s.header_s.timeStamp_w += msgBuffer[offset++];

            // Combine MDI
            if(tcpMdiFrame_s.header_s.scanSpotsNbr_w > 0)
            {
                // Find the 1st packet
                if(tcpMdiFrame_s.header_s.subNbr_b == 1)
                {
                    // Save first angle for MDI frame
                    // tcpMdiFrame_s.frame_s.startAngle_sw = tcpMdiFrame_s.header_s.firstAngle_sw;
                    tcpMdiFrame_s.frame_s.startAngle_sdw = tcpMdiFrame_s.header_s.firstAngle_sdw;
                    // Clear spots number(because not copy to Dst buffer yet)
                    tcpMdiFrame_s.frame_s.spotNbr_w = 0;
                }

                // Calculate start address index for Dst buffer
                idx = tcpMdiFrame_s.frame_s.spotNbr_w;
                for (quint16 i = 0; i < tcpMdiFrame_s.header_s.scanSpotsNbr_w;i++)
                {
                    tcpMdiFrame_s.distance_wa[idx + i] = msgBuffer[offset]*256+msgBuffer[offset+1];
                    offset += 2;
                }
                if(tcpMdiFrame_s.header_s.pktType_b == 1)
                {
                    for (quint16 i = 0; i < tcpMdiFrame_s.header_s.scanSpotsNbr_w;i++)
                    {
                        tcpMdiFrame_s.pulsewidth_wa[idx + i] = msgBuffer[offset]*256+msgBuffer[offset+1];
                        offset += 2;
                    }
                }
                // Update spots number
                tcpMdiFrame_s.frame_s.spotNbr_w += tcpMdiFrame_s.header_s.scanSpotsNbr_w;

                // Update frame angle info
                // tcpMdiFrame_s.frame_s.deltaAngle_sw = tcpMdiFrame_s.header_s.deltaAngle_sw;
                tcpMdiFrame_s.frame_s.deltaAngle_sdw = tcpMdiFrame_s.header_s.deltaAngle_sdw;

                if(tcpMdiFrame_s.header_s.subNbr_b == tcpMdiFrame_s.header_s.totalNbr_b)
                {
                    newRotation_b = 1;
                }
                curFrameTotalNbr_b = tcpMdiFrame_s.header_s.totalNbr_b;
            }

            // Increase total frame number
            if(isFirstTime_b)
            {
                isFirstTime_b = false;
                totalFrame = tcpMdiFrame_s.header_s.pktNbr_w;
            }
            else {
                totalFrame++;
            }

            // emit signal
            if(newRotation_b == 1)
            {
                emit sigReadMdiData();
            }
        }
    }
}

void TcpClient::onConnected()
{
    isConnected = true;
}

void TcpClient::onDisconnected()
{
    isConnected = false;
}
