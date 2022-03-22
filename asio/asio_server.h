#ifndef AUBO_COMM_ASIO_SERVER_H
#define AUBO_COMM_ASIO_SERVER_H

#include "asio.hpp"
#include <QDebug>

// namespace rtde {

using asio::ip::tcp;

template <class T>
class TcpSession : public std::enable_shared_from_this<TcpSession<T>>
{
public:
    using UserType = typename T::UserType;
    using UserTypePtr = std::shared_ptr<UserType>;

    TcpSession(UserTypePtr user_data, tcp::socket socket)
        : user_data_(user_data), socket_(std::move(socket))
    {
        cb_ = std::make_shared<T>();
        connected_ = true;
        asio::ip::tcp::no_delay no_delay_option(true);
        asio::socket_base::reuse_address sol_reuse_option(true);
        socket_.set_option(no_delay_option);
        socket_.set_option(sol_reuse_option);
#if defined(__linux) || defined(linux) || defined(__linux__)
        asio::detail::socket_option::boolean<IPPROTO_TCP, TCP_QUICKACK>
            quickack(true);
        socket_.set_option(quickack);
#endif
    }

    void start() { do_read(); }

    void do_read()
    {
        memset(data_.data(), 0, sizeof(data_));
        //        auto self(shared_from_this());
        auto self =
            std::enable_shared_from_this<TcpSession<T>>::shared_from_this();
        int arg[] = { 1 };
        setsockopt(socket_.native_handle(), IPPROTO_TCP, TCP_QUICKACK, arg,
                   sizeof(int));
        socket_.async_receive(
            asio::buffer(data_, max_length),
            [this, self](std::error_code ec, std::size_t length) {
                if (!ec) {
                    // 处理接收到的消息
                    // 解决TCP Stream数据包分割的问题
                    cb_->onReceive(self, data_.data(), length);

                    // 继续读取
                    do_read();
                    //                                        do_write(data_.data()
                    //                                        , length);
                    printf("------------- \n");
                } else {
                    // 断开连接之后是否需要对机器人做安全保护措施
                    ec.message();
                    connected_ = false;
                    cb_->onClose(self);
                }
            });
    }

    template <typename _T>
    void doWrite(_T &&str)
    {
        auto self =
            std::enable_shared_from_this<TcpSession<T>>::shared_from_this();

        asio::async_write(
            socket_, asio::buffer(str),
            [self, this](std::error_code error_code, std::size_t length) {
                if (error_code) {
                    connected_ = false;
                    cb_->onClose(self);
                } else {
                    // std::cout << "doWrite " << length << std::endl;
                }
            });
    }

    void do_write(char *data, std::size_t length)
    {
        //        auto self(shared_from_this());
        auto self =
            std::enable_shared_from_this<TcpSession<T>>::shared_from_this();

        asio::async_write(socket_, asio::buffer(data, length),
                          [this, self](std::error_code ec, std::size_t length) {
                              if (ec) {
                                  //                    do_read();
                                  connected_ = false;
                                  cb_->onClose(self);
                              } else {
                                  // std::cout << "doWrite " << length <<
                                  // std::endl;
                              }
                          });
    }

    bool isConnected() const { return connected_; }
    tcp::endpoint getRemoteInfo() { return socket_.remote_endpoint(); }

    std::shared_ptr<T> getCallback() { return cb_; }
    UserTypePtr getUserData() { return user_data_; }

private:
    UserTypePtr user_data_;
    bool connected_{ false };
    std::shared_ptr<T> cb_;
    tcp::socket socket_;
    enum
    {
        max_length = 1024
    };
    //    char data_[max_length];
    std::array<char, max_length> data_;
};

/**
 * @tparam T 回调函数
 */
template <class T>
class TcpServer
{
public:
    using Session = TcpSession<T>;
    using UserType = typename T::UserType;
    using UserTypePtr = std::shared_ptr<UserType>;
    using SessionPtr = std::shared_ptr<Session>;

    //    TcpServer(asio::io_context &io_context, short port)
    //        : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
    //    {
    //        do_accept();
    //    }

    TcpServer(int port, UserTypePtr user_data)
        : acceptor_(io_context_, tcp::endpoint(tcp::v4(), port)),
          user_data_(user_data)
    {
        doAccept();
    }

    //    void do_accept()
    //    {
    //        acceptor_.async_accept([this](std::error_code ec, tcp::socket
    //        socket) {
    //            if (!ec) {
    //                std::make_shared<Session>(std::move(socket))->start();
    //            }

    //            do_accept();
    //        });
    //    }
    void doAccept()
    {
        acceptor_.async_accept([this](std::error_code error_code,
                                      tcp::socket socket) {
            if (!error_code) {
                // \TODO(louwei): 需要限制一下会话的数量
                // \TODO(louwei): 还需要做身份认证，校验用户名密码
                if (sessions_.size() < 50) {
                    auto s = std::make_shared<Session>(user_data_,
                                                       std::move(socket));
                    sessions_.push_back(s);

                    qDebug() << "An RTDE session built ip "
                             << QString::fromStdString(
                                    s->getRemoteInfo().address().to_string())
                             << ":" << s->getRemoteInfo().port();

                    s->getCallback()->onConnect(s);
                    s->do_read();
                }
            }

            doAccept(); // 重新accept
        });
    }

    void update()
    {
        // 清除断开的session
        for (auto it = sessions_.begin(); it != sessions_.end();) {
            if (!(*it)->isConnected()) {
                sessions_.erase(it);
            } else {
                it++;
            }
        }

        for (auto it : sessions_) {
            if (it->isConnected()) {
                it->getCallback()->onUpdate(it);
            }
        }

        // 驱动ASIO
        io_context_.poll();
    }

private:
    asio::io_context io_context_;
    tcp::acceptor acceptor_;
    std::vector<SessionPtr> sessions_;
    UserTypePtr user_data_;
};

//} // namespace rtde

#endif // AUBO_COMM_ASIO_SERVER_H
