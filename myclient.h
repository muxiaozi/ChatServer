#ifndef MYCLIENT_H
#define MYCLIENT_H

#include <QThread>
#include <QString>
#include <QByteArray>
#include <QDebugStateSaver>

class QTcpSocket;
class MyServer;

class MyClient : public QThread
{
    Q_OBJECT
public:
    enum DataType{
        CONNECTED = 1,
        DISCONNECTED = 2,
        TEXT_TO_ONE = 3,
        TEXT_TO_ALL = 4,
        VOICE_TO_ONE = 5,
        VOICE_TO_ALL = 6,
    };

public:
    explicit MyClient(qintptr socketDescriptor, QObject *parent = Q_NULLPTR);

    qintptr getSocketDescriptor() const;
    QString getName() const;
    QTcpSocket *getSocket() const;
    void forceDisconnect();
    void sendOnlineUserToMe();
    void setServer(MyServer *server);

signals:
    void onClientConnected(qintptr user, const QString &name);
    void onClientDisconnected(qintptr user);
    void sendToOne(qintptr someone, const QByteArray &data);
    void sendExceptOne(qintptr someone, const QByteArray &data);

public slots:
    void sendData(qintptr user, const QByteArray &data);

private slots:
    void onReadyRead();
    void onDisconnected();

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QTcpSocket *socket;

    //服务端识别码，用于创建客户端
    qintptr socketDescriptor;

    QString name;
    QByteArray packetData;

    MyServer *server;

};



#endif // MYCLIENT_H
