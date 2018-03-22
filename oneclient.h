#ifndef ONECLIENT_H
#define ONECLIENT_H

#include <QThread>
#include <QByteArray>

class QTcpSocket;

class OneClient : public QThread
{
    Q_OBJECT
public:
    explicit OneClient(quint64 id,const QTcpSocket *socket, const QString &name, QObject *parent = Q_NULLPTR);

    quint64 getId() const;
    QString &getName() const;
    void sendData(const QByteArray &data);

signals:
    void receiveData(const QByteArray &data);

protected:
    void run() Q_DECL_OVERRIDE;

private:
    QTcpSocket *socket;
    QString name;
    quint64 id;
};

#endif // ONECLIENT_H
