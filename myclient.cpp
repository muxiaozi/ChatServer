#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "myclient.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QByteArray>
#include <QDebug>
#include "myserver.h" //为了获取客户端列表

MyClient::MyClient(qintptr socketDescriptor, QObject *parent) :
    QThread(parent)
{
    this->socketDescriptor = socketDescriptor;
}

qintptr MyClient::getSocketDescriptor() const
{
    return socketDescriptor;
}

QString MyClient::getName() const
{
    return name;
}

QTcpSocket *MyClient::getSocket() const
{
    return socket;
}

void MyClient::forceDisconnect()
{
    socket->disconnectFromHost();
}

/**
 * @brief 把在线列表发送给自己
 */
void MyClient::sendOnlineUserToMe()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);

    QList<MyClient*> clients = server->getClientList();
    for(auto client : clients){
        stream.device()->seek(0);
        data.clear();
        stream<<(int)0;
        stream<<(int)CONNECTED;
        stream<<client->getSocketDescriptor();
        stream<<client->getName().toUtf8();
        stream.device()->seek(0);
        stream<<data.size();
        socket->write(data);
    }
}

void MyClient::setServer(MyServer *server)
{
    this->server = server;
}

void MyClient::sendData(qintptr user, const QByteArray &data)
{
    if(user == getSocketDescriptor()){
        socket->write(data);
    }
}

void MyClient::onReadyRead()
{
    int size;           //大小
    int type;           //类型
    qintptr receiver;   //接收/发送方
    QByteArray tmpData; //数据包内容

    packetData.append(socket->readAll());
    QDataStream stream(&packetData, QIODevice::ReadWrite);

    while(packetData.size() > sizeof(int)){
        stream>>size;
        if(packetData.size() < size) break;
        stream>>type;
        stream>>receiver;

        //接收方id -> 发送方id
        stream.device()->seek(stream.device()->pos() - sizeof(qintptr));
        stream<<socketDescriptor;

        qDebug()<<"receiver: "<<receiver<<", type: "<< type;

        switch(type){
        case CONNECTED: //用户信息
        {
            stream>>tmpData;
            name = QString::fromUtf8(tmpData);

            sendOnlineUserToMe();

            emit onClientConnected(getSocketDescriptor(), getName());
            emit sendExceptOne(getSocketDescriptor(), packetData.left(size));
            break;
        }
        case TEXT_TO_ONE:
        case VOICE_TO_ONE:
        {
            emit sendToOne(receiver, packetData.left(size));
            break;
        }
        case TEXT_TO_ALL:
        case VOICE_TO_ALL:
        case DISCONNECTED:
        {
            emit sendExceptOne(getSocketDescriptor(), packetData.left(size));
            break;
        }
        default:
            //如果不能识别，可能连接的不是客户端，直接断开连接
            socket->disconnectFromHost();
            return;
        }

        //截取未读数据块
        packetData = packetData.right(packetData.size() - size);
        stream.device()->seek(0);
    }
}

void MyClient::onDisconnected()
{
    qDebug()<<"线程结束 "<<QThread::currentThread();

    //构造自己退出信息，发送给所有人
    QByteArray data;
    QDataStream stream(&packetData, QIODevice::WriteOnly);
    stream<<(int)0;
    stream<<(int)DISCONNECTED;
    stream<<getSocketDescriptor();
    stream.device()->seek(0);
    stream<<data.size();
    emit onClientDisconnected(getSocketDescriptor());
    emit sendExceptOne(getSocketDescriptor(), data);

    quit();
}

void MyClient::run()
{
    qDebug()<<"创建新线程 "<<QThread::currentThread();
    socket = new QTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

    //使线程进入自己的事件循环
    exec();
}
