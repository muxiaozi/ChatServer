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
        CONNECTED = 1,
        DISCONNECTED = 2,
        TEXT_TO_ONE = 3,
        TEXT_TO_ALL = 4,
        VOICE_TO_ONE = 5,
        VOICE_TO_ALL = 6,
    };

public:
    explicit MyClient(qintptr socketDescriptor, QObject *parent = Q_NULLPTR);
    ~MyClient();
    void sendOnlineUserToMe(qintptr user);
    qintptr getSocketDescriptor() const;
    QString getName() const;

signals:
    void clientConnected(qintptr user, const QString& name);
    void sendToOne(qintptr user, const QByteArray &data);
    void sendExceptOne(qintptr user, const QByteArray &data);

public slots:
    void onSendData(qintptr user, const QByteArray &data);
    void onForceDisconnect(qintptr user);

private slots:
    void onReadyRead();
    void onDisconnected();

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QTcpSocket *socket;         //socket对象
    qintptr socketDescriptor;   //连接描述符
    QByteArray cacheData;       //数据缓存
    QString name;               //客户端昵称

};



#endif // MYCLIENT_H
