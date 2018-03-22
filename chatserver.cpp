#include "chatserver.h"
#include "ui_chatserver.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QLabel>
#include <QMessageBox>
#include <QDataStream>
#include <QByteArray>
#include <QJsonObject>

ChatServer::ChatServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatServer)
{
    ui->setupUi(this);

    //设置表格 名称 登陆时间 操作
    ui->table->setColumnCount(3);

    QStringList header;
    header<<tr("操作")<<tr("名称")<<tr("登陆时间");
    ui->table->setHorizontalHeaderLabels(header);

    ui->table->horizontalHeader()->setStretchLastSection(true);
    ui->table->horizontalHeader()->sectionResizeMode(QHeaderView::Stretch);

    QFont font = ui->table->horizontalHeader()->font();
    font.setBold(true);
    ui->table->horizontalHeader()->setFont(font);

    //初始化服务器
    server = new QTcpServer;
    server->listen(QHostAddress::AnyIPv4, 10086);
    connect(server, SIGNAL(newConnection()), SLOT(newConnectionSlot()));
    setWindowTitle(windowTitle() + QString(" - 端口: %1").arg(server->serverPort()));
    ui->statusBar->showMessage("就绪");
}

ChatServer::~ChatServer()
{
    delete ui;
    delete server;
}

void ChatServer::insertNewRow(quint64 id, const QString &userName)
{
    int row = ui->table->rowCount();
    ui->table->insertRow(row);

    QTableWidgetItem *del = new QTableWidgetItem(QIcon(":/images/close.png"),tr("强制下线"));
    del->setFlags(Qt::ItemIsEnabled);
    del->setData(Qt::UserRole, id);
    del->setTextColor(QColor(Qt::red));
    ui->table->setItem(row, 0, del);

    QTableWidgetItem *name = new QTableWidgetItem(userName);
    name->setFlags(Qt::ItemIsEnabled);
    ui->table->setItem(row, 1, name);

    QTableWidgetItem *time = new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    time->setFlags(Qt::ItemIsEnabled);
    ui->table->setItem(row, 2, time);
}

void ChatServer::removeRow(quint64 id)
{
    for(int i = 0; i < ui->table->rowCount(); ++i){
        quint64 _id = ui->table->item(i, 0)->data(Qt::UserRole).toULongLong();
        if(_id == id){
            ui->table->removeRow(i);
            break;
        }
    }
}

void ChatServer::sendToOne(quint64 id, const QByteArray &data)
{
    for(auto entity : clientList){
        if(entity.id == id){
            entity.socket->write(data);
            break;
        }
    }
}

void ChatServer::sendExceptOne(quint64 id, const QByteArray &data)
{
    for(auto entity : clientList){
        if(entity.id != id){
            entity.socket->write(data);
        }
    }
}


void ChatServer::onClientConnected(const QByteArray &data)
{
    //读取ID以及姓名
    connect(socket, SIGNAL(disconnected()), SLOT(clientDisconnected()));
    quint64 id = socket->read(sizeof(quint64)).toULongLong();
    ClientEntity clientEntity;
    clientEntity.socket = socket;
    clientEntity.name = QString::fromUtf8(socket->readLine());

    clientMap.insert(id, clientEntity);
    insertNewRow(id, clientEntity.name);
    ui->statusBar->showMessage(QString("当前在线人数: %1").arg(clientMap.size()));
}

void ChatServer::newConnectionSlot()
{
    //接收客户端连接
    QTcpSocket *socket = server->nextPendingConnection();
    connect(socket, SIGNAL(readyRead()), SLOT(readySlot()));
}

void ChatServer::receiveData(QByteArray &data)
{
    MyMessage msg;
    stream>>(qint32)msg.type;
    stream>>(quint64)msg.senderId;

    switch(msg.type){
        case MyMessage::CLIENT_CONNECTED:
        {
            QString name;
            stream>>name;
            sendExceptOne(senderId, stream.device()->readLineData());
            break;
        }
        case RECEIVE_TEXT_SINGAL:
        {
            QString msg;
            stream>>msg;

            sendToOne();

            break;
        }
        case RECEIVE_TEXT_ALL:
        {
            break;
        }
        default:
            break;
    }
    QMutexLocker locker(&mutex);

    messages.append();
}

void ChatServer::readySlot()
{
    QTcpSocket *socket = (QTcpSocket*)sender();

    if(socket->bytesAvailable() > sizeof(quint32)){
        quint32 blockSize = socket->read(sizeof(quint32)).toUInt();
        while(socket->bytesAvailable() < blockSize){
            //如果连接被断开，则取消接收数据
            if(!socket->isOpen()) return;
        }
        QByteArray data = socket->read(blockSize);
    }
    while(socket->bytesAvailable() > 0){
        data.append(socket->readAll());
    }

    QDataStream stream(data);
    while(!stream.atEnd()){
        //一个用户同时发送了多组数据
        //需要多次解析
        transmitData(stream);
    }
}

void ChatServer::clientDisconnected()
{
    QTcpSocket *socket = (QTcpSocket*)sender();
    for(int i = 0; i < clientList.length(); ++i){
        if(clientList.at(i).socket == socket){
            removeRow(clientList.at(i).id);
            clientList.removeAt(i);
            break;
        }
    }
    ui->statusBar->showMessage(QString("当前在线人数: %1").arg(clientList.length()));
}

void ChatServer::on_table_clicked(const QModelIndex &index)
{
    if(index.column() == 0){
        if(QMessageBox::warning(this, tr("警告"), tr("确定要强制下线该用户？"),
                                QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes){
            QTcpSocket *s = ui->table->item(index.row(), index.column())->data(Qt::UserRole)
                    .value<QTcpSocket*>();
            s->disconnectFromHost();
        }
    }
}
