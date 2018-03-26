#include "myclient.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QByteArray>
#include <QDebug>

MyClient::MyClient(qintptr socketDescriptor, QObject *parent) :
    QThread(parent),
    socketDescriptor(socketDescriptor)
{
}

qintptr MyClient::getSocketDescriptor()
{
    return clientDescriptor;
}

QString MyClient::getName()
{
    return name;
}

void MyClient::sendData(const QByteArray &data)
{
    if(socket->isWritable()){
        socket->write(data);
    }
}

void MyClient::forceDisconnect()
{
    socket->disconnectFromHost();
}

void MyClient::onReadyRead()
{
    int size;
    int type;
    qintptr sender;

    packetData.append(socket->readAll());
    QDataStream stream(&packetData, QIODevice::ReadOnly);

    stream>>size;
    if(packetData.size() < size)
        return;
    stream>>type;
    stream>>sender;

    switch(type){
        case CLIENT_CONNECTED: //用户信息
        {
            QByteArray info;
            stream>>info;
            clientDescriptor = sender;
            name = QString::fromUtf8(info);
            emit onClientConnected(sender, name);
            emit sendExceptOne(sender, packetData.left(size));
            break;
        }
        case RECEIVE_TEXT_SINGAL:
        case RECEIVE_VOICE_SINGAL:
        {
            qintptr receiver;
            stream>>receiver;
            QByteArray byteText;
            stream>>byteText;
            qDebug()<<QString::fromUtf8(byteText);
            emit sendToOne(receiver, packetData.left(size));
            break;
        }
        case CLIENT_DISCONNECTED:
        case RECEIVE_TEXT_ALL:
        case RECEIVE_VOICE_ALL:
        {
            emit sendExceptOne(sender, packetData.left(size));
            break;
        }
        default:
            //如果不能识别，可能连接的不是客户端，直接断开连接
            socket->disconnectFromHost();
            return;
    }

    packetData = packetData.right(packetData.size() - size);
}

void MyClient::onDisconnected()
{
    qDebug()<<"线程结束 "<<socketDescriptor;

    //构造自己退出信息，发送给所有人
    QByteArray data;
    QDataStream stream(&packetData, QIODevice::WriteOnly);
    stream<<(int)0;
    stream<<(int)CLIENT_DISCONNECTED;
    stream<<getSocketDescriptor();
    stream.device()->seek(0);
    stream<<data.size();
    emit onClientDisconnected(getSocketDescriptor());
    emit sendExceptOne(getSocketDescriptor(), data);

    quit();
}

void MyClient::run()
{
    qDebug()<<"创建新线程 "<<socketDescriptor;

    socket = new QTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, SIGNAL(readyRead()), SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), SLOT(onDisconnected()));

    //使线程进入自己的事件循环
    exec();
}
