#include "myserver.h"
#include "myclient.h"
#include <QDebug>

MyServer::MyServer(QObject *parent) :
    QTcpServer(parent)
{
}

QList<MyClient *> MyServer::getClientList() const
{
    return clientList;
}

void MyServer::onSendToOne(qintptr someone, const QByteArray &data)
{
    for(auto client : clientList){
        if(client->getSocketDescriptor() == someone){
            client->sendData(data);
            break;
        }
    }
}

void MyServer::onSendExceptOne(qintptr someone, const QByteArray &data)
{
    for(auto client : clientList){
        if(client->getSocketDescriptor() != someone){
            client->sendData(data);
            break;
        }
    }
}

void MyServer::clientConnected(qintptr user, const QString &name)
{

    emit onClientConnected(user, name);
}

void MyServer::onClientFinished()
{
    MyClient *finishedClient = (MyClient*) sender();
    clientList.removeOne(finishedClient);
    finishedClient->deleteLater();
}

void MyServer::forceDisconnect(qintptr client)
{
    for(auto c : clientList){
        if(c->getSocketDescriptor() == client){
            c->forceDisconnect();
            break;
        }
    }
}

void MyServer::incomingConnection(qintptr handle)
{
    MyClient *client = new MyClient(handle, this);

    //线程退出通知
    connect(client, SIGNAL(finished()), this, SLOT(onClientFinished()));

    //客户端连接
    connect(client, SIGNAL(onClientConnected(qintptr,QString)),
            this, SLOT(clientConnected(qintptr,QString)));

    //客户端断开
    connect(client, SIGNAL(onClientDisconnected(qintptr)),
            this, SIGNAL(onClientDisconnected(qintptr)));

    //有消息需要发送给某一个人
    connect(client, SIGNAL(sendToOne(qintptr,QByteArray)),
            this, SLOT(onSendToOne(qintptr,QByteArray)), Qt::QueuedConnection);

    //有消息需要发送给除了某一个人以外的所有人
    connect(client, SIGNAL(sendExceptOne(qintptr,QByteArray)),
            this, SLOT(onSendExceptOne(qintptr,QByteArray)), Qt::QueuedConnection);

    client->start();
}
