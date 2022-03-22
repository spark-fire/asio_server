#include <QCoreApplication>
#include "asio/asio_server.h"

#include "rtde/rtde_service.h"
#include "rtde/rtde_asio.h"

#include <unistd.h>

// using rtde;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    int rtde_tcp_port = 12306;
    std::shared_ptr<RtdeAsio> rtde_asio_{ nullptr };

    //    asio::io_context io_context;

    //    server<int> s(io_context, 12306);

    //    io_context.run();

    auto rtde_service = std::make_shared<RtdeService>();

    rtde_asio_ = std::make_shared<RtdeAsio>(rtde_tcp_port, rtde_service);

    //    // 打包所有的菜单
    //    rtde_service->packAll();

    while (1) {
        printf(" I am in main !  heat ...... \n");
        sleep(3);
        rtde_asio_->update();
    }

    return a.exec();
}
