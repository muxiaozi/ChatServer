#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QMainWindow>

namespace Ui {
    class ChatServer;
}

class MyServer;

class ChatServer : public QMainWindow
{
    Q_OBJECT
public:
    explicit ChatServer(QWidget *parent = 0);
    ~ChatServer();

private slots:
    void on_table_clicked(const QModelIndex &index);
    void onClientConnected(qintptr user, const QString &name);
    void onClientDisconnected(qintptr user);

private:
    Ui::ChatServer *ui;
    MyServer *server;
};

#endif // CHATSERVER_H
