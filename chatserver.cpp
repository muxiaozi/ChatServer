#include "chatserver.h"
#include "ui_chatserver.h"
#include "myserver.h"

#include <QTimer>
#include <QDebug>
#include <QDateTime>
#include <QLabel>
#include <QMessageBox>

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
    server = new MyServer(this);
    server->listen(QHostAddress::AnyIPv4, 10086);
    connect(server, SIGNAL(onClientConnected(qintptr,QString)),
            this, SLOT(onClientConnected(qintptr,QString)));
    connect(server, SIGNAL(onClientDisconnected(qintptr)),
            this, SLOT(onClientDisconnected(qintptr)));

    setWindowTitle(windowTitle() + QString(" - 端口: %1").arg(server->serverPort()));
    ui->statusBar->showMessage("就绪");
}

ChatServer::~ChatServer()
{
    delete ui;
    delete server;
}

void ChatServer::on_table_clicked(const QModelIndex &index)
{
    if(index.column() == 0){
        if(QMessageBox::warning(this, tr("警告"), tr("确定要强制下线该用户？"),
                                QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes){
            server->forceDisconnect(ui->table->item(index.row(), 1)
                                    ->data(Qt::UserRole).value<qintptr>());
        }
    }
}

void ChatServer::onClientConnected(qintptr user, const QString &name)
{
    int row = ui->table->rowCount();
    ui->table->insertRow(row);

    QTableWidgetItem *offlineItem = new QTableWidgetItem(QIcon(":/images/close.png"),tr("强制下线"));
    offlineItem->setFlags(Qt::ItemIsEnabled);
    offlineItem->setTextColor(QColor(Qt::red));
    ui->table->setItem(row, 0, offlineItem);

    QTableWidgetItem *nameItem = new QTableWidgetItem(name);
    nameItem->setData(Qt::UserRole, QVariant::fromValue<qintptr>(user));
    nameItem->setFlags(Qt::ItemIsEnabled);
    ui->table->setItem(row, 1, nameItem);

    QTableWidgetItem *timeItem = new QTableWidgetItem(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    timeItem->setFlags(Qt::ItemIsEnabled);
    ui->table->setItem(row, 2, timeItem);
}

void ChatServer::onClientDisconnected(qintptr user)
{
    for(int i = 0; i < ui->table->rowCount(); ++i){
        qintptr id = ui->table->item(i, 1)->data(Qt::UserRole).value<qintptr>();
        if(user == id){
            ui->table->removeRow(i);
            break;
        }
    }
}
