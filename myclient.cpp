#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif

#include "myclient.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QDebug>

MyClient::MyClient(qintptr socketDescriptor, QObject *parent) :
    QThread(parent),
    socketDescriptor(socketDescriptor)
{
    qDebug()<<"Welcome "<<socketDescriptor;
}

MyClient::~MyClient()
{
    qDebug()<<"Bye "<<socketDescriptor;
}

qintptr MyClient::getSocketDescriptor() const
{
    return socketDescriptor;
}

QString MyClient::getName() const
{
    return name;
}

void MyClient::onSendData(qintptr user, const QByteArray &data)
{
    if(user == socketDescriptor){
        qDebug()<<name<<" send: "<<data;
        socket->write(data);
    }
}

void MyClient::onForceDisconnect(qintptr user)
{
    if(user == socketDescriptor){
        socket->disconnectFromHost();
    }
}

void MyClient::onReadyRead()
{
    int size;           //大小
    int type;           //类型
    qintptr receiver;   //接收者
    QByteArray data;    //数据包内容

    cacheData.append(socket->readAll());
    QDataStream stream(&cacheData, QIODevice::ReadWrite);

    while(cacheData.size() > sizeof(int)){
        //大小
        stream>>size;
        if(cacheData.size() < size) break;

        //类型
        stream>>type;

        //用户ID
        stream>>receiver;
        stream.device()->seek(stream.device()->pos() - sizeof(qintptr));
        stream<<socketDescriptor;

        //数据
        stream>>data;

        //处理
        switch(type){
        case CONNECTED:
            name = QString::fromUtf8(data);
            emit clientConnected(socketDescriptor, name);
            emit sendExceptOne(socketDescriptor, cacheData.left(size));
            break;
        case TEXT_TO_ALL:
        case VOICE_TO_ALL:
            emit sendExceptOne(socketDescriptor, cacheData.left(size));
            break;
        case TEXT_TO_ONE:
        case VOICE_TO_ONE:
            emit sendToOne(receiver, cacheData.left(size));
            break;
        default:
            break;
        }

        //截取未读数据块
        cacheData = cacheData.right(cacheData.size() - size);
        stream.device()->seek(0);
    }
}

void MyClient::onDisconnected()
{
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream<<(int)0;
    stream<<(int)DISCONNECTED;
    stream<<socketDescriptor;
    stream.device()->seek(0);
    stream<<data.size();
    emit sendExceptOne(socketDescriptor, data);

    quit();
}

void MyClient::run()
{
    socket = new QTcpSocket();
    socket->setSocketDescriptor(socketDescriptor);

    connect(socket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));

    //进入线程事件循环
    exec();
}


