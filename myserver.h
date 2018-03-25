﻿#ifndef MYSERVER_H
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
    void onClientConnected(qintptr user, const QString &name);
    void onClientDisconnected(qintptr user);

public slots:
    void onSendToOne(qintptr someone, const QByteArray &data);
    void onSendExceptOne(qintptr someone, const QByteArray &data);
    void onClientFinished();

protected:
    void incomingConnection(qintptr handle) Q_DECL_OVERRIDE;

private:
    QList<MyClient*> clientList;
};

#endif // MYSERVER_H