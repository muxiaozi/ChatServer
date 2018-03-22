#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QMainWindow>
#include <QList>
#include <QMutex>
#include <QQueue>

namespace Ui {
    class ChatServer;
}

class QTcpServer;
class QTcpSocket;
class QDataStream;
class OneClient;


class ChatServer : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * 客户端数据结构
     * 客户端列表中包含的节点类型
     */
    struct ClientEntity{
        quint64 id;         //ID
        OneClient *client;  //socket
    };

    /**
     * @brief 解析数据包
     */
    struct MyMessage{
        enum DataType{
            CLIENT_CONNECTED = 0,       // id, name     [quint64, utf8]
            CLIENT_DISCONNECTED = 1,    // id           [quint64]
            RECEIVE_TEXT_SINGAL = 2,    // id, text     [quint64, utf8]
            RECEIVE_TEXT_ALL = 3,       // id, text     [quint64, utf8]
            RECEIVE_VOICE_SINGAL = 4,   // id, voice    [quint64, quint32, blob]
            RECEIVE_VOICE_ALL = 5,      // id, voice    [quint64, quint32, blob]
        }type;

        quint64 senderId;
        quint64 receiverId;
        union{
            QString s;
            QByteArray b;
        }data;
    };

public:
    explicit ChatServer(QWidget *parent = 0);
    ~ChatServer();

    void insertNewRow(quint64 id, const QString &userName);
    void removeRow(quint64 id);
    void sendToOne(quint64 id, const QByteArray &data);
    void sendExceptOne(quint64 id, const QByteArray &data);

    void onClientConnected(const QByteArray &data);

private slots:
    void newConnectionSlot();
    void receiveData(QByteArray &data);
    void clientDisconnected();

    void on_table_clicked(const QModelIndex &index);

private:
    Ui::ChatServer *ui;
    QTcpServer *server;

    QList<ClientEntity> clientList;
    QQueue<MyMessage> messages;
    QMutex mutex;
};

#endif // CHATSERVER_H
