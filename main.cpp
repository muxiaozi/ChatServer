#include "chatserver.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<qintptr>("qintptr");

    QApplication a(argc, argv);
    ChatServer w;
    w.show();

    return a.exec();
}
