#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QThread>
#include <QTcpSocket>
#include <QHostAddress>
#include <QMutex>
#include <QByteArray>
#include <QTimer>

class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient();
    ~TcpClient();

    void startConnect(const QString& strAddressIP, quint16 iPort);

    void write(quint8 *addr, quint16 size);
    int read(QByteArray &bufferIn);
    void close(void);

    QTcpSocket *m_TcpSocket;

signals:
    void signalReadData();

public slots:
    void onConnect();
    void onDisconnect();
    void onErrorString(QAbstractSocket::SocketError errorString);

    void onRecvData();
    //void onSendData();

private:
    QHostAddress m_hostaddressIP;
    QString m_strAddressIP;
    quint16 m_iPort;
    bool b_isConnectState;
    bool b_hasDataSend;
    bool b_close;

    QTimer *m_timerConnect;

    QByteArray m_readDataBuf;
    QByteArray m_sendDatabuf;
};


#endif //TCPCLIENT_H
