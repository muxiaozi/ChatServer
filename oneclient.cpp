#include "oneclient.h"
#include <QTcpSocket>
#include <QDataStream>
#include <QByteArray>

OneClient::OneClient(quint64 id, const QTcpSocket *socket,
                     const QString &name, QObject *parent)
    : id(id), socket(socket), name(name), QThread(parent)
{
}

quint64 OneClient::getId() const
{
    return id;
}

QString &OneClient::getName() const
{
    return name;
}

void OneClient::sendData(const QByteArray &data)
{
    socket->write(data);
}

void OneClient::run()
{
    while(socket->isOpen()){
        while(socket->bytesAvailable() < sizeof(quint32));
        quint32 blockSize = socket->read(sizeof(quint32));
        while(socket->bytesAvailable() < blockSize);
        QByteArray data;
        QDataStream stream(data);
        stream<<blockSize;
        stream<<socket->read((blockSize);

        emit receiveData(data);
    }
}
