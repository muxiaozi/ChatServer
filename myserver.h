#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>
#include <QList>

class MyClient;

class MyServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit MyServer(QObject *parent = Q_NULLPTR);

signals:
    void clientConnected(qintptr user, const QString &name);
    void clientDisconnected(qintptr user);
    void sendData(qintptr user, const QByteArray &data);
    void forceDisconnect(qintptr user);

public slots:
    void sendToOne(qintptr someone, const QByteArray &data);
    void sendExceptOne(qintptr someone, const QByteArray &data);
    void onClientConnected(qintptr user, const QString &name);
    void onClientDisconnected();
    void onForceDisconnect(qintptr client);

protected:
    void incomingConnection(qintptr handle) Q_DECL_OVERRIDE;

private:
    QList<MyClient*> clientList;
};

#endif // MYSERVER_H
