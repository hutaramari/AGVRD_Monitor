#include "tcpClient.h"

#include <QtWidgets>


TcpClient::TcpClient()
    : b_isConnectState(false)
    , m_strAddressIP("127.0.0.1"), m_iPort(0)
{
    m_TcpSocket = new QTcpSocket();

    connect(m_TcpSocket, SIGNAL(connected()), this, SLOT(onConnect()));
    connect(m_TcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnect()));
    connect(m_TcpSocket, SIGNAL(readyRead()), this, SLOT(onRecvData()));
    connect(m_TcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onErrorString(QAbstractSocket::SocketError)), Qt::DirectConnection);

    m_timerConnect = new QTimer();
    connect(m_timerConnect, &QTimer::timeout, [=](){
        if(b_isConnectState)
            return;
        if(m_TcpSocket->state() != QAbstractSocket::UnconnectedState)
            return;
        m_TcpSocket->connectToHost(m_strAddressIP, m_iPort);
    });

    b_hasDataSend = false;
    b_close = false;
}

TcpClient::~TcpClient()
{
    m_TcpSocket->abort();
}

void TcpClient::startConnect(const QString &strAddressIP, quint16 iPort)
{
    m_strAddressIP = strAddressIP;
    m_iPort = iPort;
    m_timerConnect->start(1000);
}

void TcpClient::onConnect()
{
    b_isConnectState = true;
    qDebug() << "onConnect";

    m_timerConnect->stop();
}

void TcpClient::onDisconnect()
{
    if(!b_close)
    {
        b_isConnectState = false;
        qDebug()<<"onDisconnect";
        m_timerConnect->start(1000);
    }
}

void TcpClient::onErrorString(QAbstractSocket::SocketError errorString)
{
    qDebug()<<errorString;
}

void TcpClient::onRecvData()
{
    if(m_TcpSocket->state() != QAbstractSocket::ConnectedState)
        return;
    if(m_TcpSocket->bytesAvailable() < 0)
    {
        m_TcpSocket->disconnectFromHost();
        m_TcpSocket->waitForDisconnected(3000);
    }
    while(m_TcpSocket->bytesAvailable() > 0)
    {
        m_readDataBuf.resize(static_cast<int>(m_TcpSocket->bytesAvailable()));
        m_TcpSocket->read(m_readDataBuf.data(), m_readDataBuf.size());
        //QString str_tcp_recv = QString::fromLocal8Bit((m_readDataBuf));
        //qDebug() << str_tcp_recv;
        emit signalReadData();
    }
}

void TcpClient::write(quint8 buf[], quint16 size)
{
    if(m_TcpSocket->state() != QAbstractSocket::ConnectedState)
        return;

    for (int i = 0; i < size; ++i) {
        m_sendDatabuf.append(static_cast<char>(buf[i]));
    }

    qint64 iSendLen = m_TcpSocket->write(m_sendDatabuf, size);
    if(!m_TcpSocket->flush() || iSendLen < 0)
    {
        m_TcpSocket->disconnectFromHost();
        m_TcpSocket->waitForDisconnected(3000);
    }
}

int TcpClient::read(QByteArray &bufferIn)
{
    memcpy(bufferIn.data(), m_readDataBuf.data(), m_readDataBuf.size());

    return m_readDataBuf.size();
}

void TcpClient::close()
{
    b_close = true;
    qDebug()<<"Closed.";
    m_timerConnect->stop();
    m_TcpSocket->close();
}

#if 0
void TcpClient::onSendData()
{
    if(m_TcpSocket->state() != QAbstractSocket::ConnectedState)
        return;

    qint64 iSendLen = m_TcpSocket->write(m_sendDatabuf, m_sendSize);
    if(!m_TcpSocket->flush() || iSendLen < 0)
    {
        m_TcpSocket->disconnectFromHost();
        m_TcpSocket->waitForDisconnected(3000);
    }
}
#endif
