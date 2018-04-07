#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "myserver.h"
#include "myclient.h"
#include <QDebug>

MyServer::MyServer(QObject *parent) :
    QTcpServer(parent)
{
}

void MyServer::sendToOne(qintptr someone, const QByteArray &data)
{
    emit sendData(someone, data);
}

void MyServer::sendExceptOne(qintptr someone, const QByteArray &data)
{
    for(auto client : clientList){
        if(client->getSocketDescriptor() != someone){
            emit sendData(client->getSocketDescriptor(), data);
        }
    }
}

void MyServer::onClientConnected(qintptr user, const QString &name)
{
    //通知界面
    emit clientConnected(user, name);

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    for(MyClient* client : clientList){
        if(client->getSocketDescriptor() != user){
            stream.device()->seek(0);
            data.clear();
            stream<<(int)0;
            stream<<(int)MyClient::CONNECTED;
            stream<<client->getSocketDescriptor();
            stream<<client->getName().toUtf8();
            stream.device()->seek(0);
            stream<<data.size();

            emit sendToOne(user, data);
        }
    }
}

void MyServer::onClientDisconnected()
{
    MyClient *client = (MyClient*) sender();

    //通知界面
    emit clientDisconnected(client->getSocketDescriptor());

    clientList.removeOne(client);

    delete client;
}

void MyServer::onForceDisconnect(qintptr client)
{
    emit forceDisconnect(client);
}

void MyServer::incomingConnection(qintptr handle)
{
    MyClient *client = new MyClient(handle);
    client->moveToThread(client);
    clientList.append(client);

    //客户端退出
    connect(client, SIGNAL(finished()), this, SLOT(onClientDisconnected()));

    //发送消息给一个人
    connect(client, SIGNAL(sendToOne(qintptr,QByteArray)),
            this, SLOT(sendToOne(qintptr,QByteArray)));

    //发送消息给其他人
    connect(client, SIGNAL(sendExceptOne(qintptr,QByteArray)),
            this, SLOT(sendExceptOne(qintptr,QByteArray)));

    //新客户端连接
    connect(client, SIGNAL(clientConnected(qintptr,QString)),
            this, SLOT(onClientConnected(qintptr,QString)));

    //发送消息
    connect(this, SIGNAL(sendData(qintptr,QByteArray)),
            client, SLOT(onSendData(qintptr,QByteArray)));

    //强制退出
    connect(this, SIGNAL(forceDisconnect(qintptr)),
            client, SLOT(onForceDisconnect(qintptr)));

    client->start();   
}
