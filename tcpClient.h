#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMutex>
#include <QByteArray>
#include <QTimer>

class TcpClient : QObject
{
    Q_OBJECT

public:
    TcpClient();
    ~TcpClient();

    void startConnect(const QString& strAddressIP, quint16 iPort);

    void write(quint8 *addr, quint16 size);

signals:
    //void sigGotReadData();

public slots:
    void onConnect();
    void onDisconnect();
    void onErrorString(QAbstractSocket::SocketError errorString);

    void onRecvData();
    //void onSendData();

private:
    QTcpSocket *m_TcpSocket;
    QHostAddress m_hostaddressIP;
    QString m_strAddressIP;
    quint16 m_iPort;
    bool b_isConnectState;
    bool b_hasDataSend;

    QTimer *m_timerConnect;

    QByteArray m_readDataBuf;
    QByteArray m_sendDatabuf;
};


#endif //TCPCLIENT_H
