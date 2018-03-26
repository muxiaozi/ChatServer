#ifndef MYCLIENT_H
#define MYCLIENT_H

#include <QThread>
#include <QString>
#include <QByteArray>

class QTcpSocket;

class MyClient : public QThread
{
    Q_OBJECT
public:
    enum DataType{
        CLIENT_CONNECTED = 1,
        CLIENT_DISCONNECTED = 2,
        RECEIVE_TEXT_SINGAL = 3,
        RECEIVE_TEXT_ALL = 4,
        RECEIVE_VOICE_SINGAL = 5,
        RECEIVE_VOICE_ALL = 6,
    };

public:
    explicit MyClient(qintptr socketDescriptor, QObject *parent = Q_NULLPTR);

    qintptr getSocketDescriptor();
    QString getName();
    void sendData(const QByteArray &data);
    void forceDisconnect();

signals:
    void onClientConnected(qintptr user, const QString &name);
    void onClientDisconnected(qintptr user);
    void sendToOne(qintptr someone, const QByteArray &data);
    void sendExceptOne(qintptr someone, const QByteArray &data);

private slots:
    void onReadyRead();
    void onDisconnected();

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QTcpSocket *socket;

    //服务端识别码，用于创建客户端
    qintptr socketDescriptor;

    //客户端识别码，用于识别唯一客户
    qintptr clientDescriptor;

    QString name;
    QByteArray packetData;

};

#endif // MYCLIENT_H
