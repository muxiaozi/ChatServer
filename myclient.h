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
        CLIENT_CONNECTED = 1,       // id, name     [quint64, utf8]
        CLIENT_DISCONNECTED = 2,    // id           [quint64]
        RECEIVE_TEXT_SINGAL = 3,    // id, text     [quint64, utf8]
        RECEIVE_TEXT_ALL = 4,       // id, text     [quint64, utf8]
        RECEIVE_VOICE_SINGAL = 5,   // id, voice    [quint64, quint32, blob]
        RECEIVE_VOICE_ALL = 6,      // id, voice    [quint64, quint32, blob]
    };

public:
    explicit MyClient(qintptr socketDescriptor, QObject *parent = Q_NULLPTR);

    qintptr getSocketDescriptor();
    QString getName();
    void sendData(const QByteArray &data);

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
