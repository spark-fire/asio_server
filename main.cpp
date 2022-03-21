#include <QCoreApplication>
#include "asio/asio_server.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    asio::io_context io_context;

    server<int> s(io_context, 12306);

    io_context.run();

    return a.exec();
}
