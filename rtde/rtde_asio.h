#include "asio/asio_server.h"
#include "rtde_service.h"

using asio::ip::tcp;

class RTDECallback : public RtdeCallback
{
public:
    using UserType = RtdeService;
    using Session = session<RTDECallback>;
    using SessionPtr = std::shared_ptr<Session>;

    RTDECallback() : RtdeCallback() {}

    ~RTDECallback() {}
    int onConnect(SessionPtr session)
    { // Output::actual_q;
        // 连接之后需要直接反馈菜单给用户
        //        LOGGING(INFO) << "An RTDE session built ip "
        //                      <<
        //                      session->getRemoteInfo().address().to_string()
        //                      << ":"
        //                      << session->getRemoteInfo().port();
        Json j;
        j["protocol"] = "RTDE";
        j["version"] = 1;

        auto service_ = session->getUserData();
        j["input"] = service_->getInputMap();
        j["output"] = service_->getOutputMap();
        auto str = j.dump();
        str.append("\n\n");

        session->doWrite(str);

        return 0;
    }

    int onUpdate(SessionPtr session)
    {
        auto service_ = session->getUserData();
        return RtdeCallback::update(service_,
                                    [session](char *data, size_t len) {
                                        if (len) {
                                            session->do_write(data, len);
                                        }
                                    });
    }
    void onReceive(SessionPtr session, char *data, size_t len)
    {
        // LOGGING(DEBUG) << "onReceive " << std::string(data, len) <<
        // std::endl;
        auto service_ = session->getUserData();
        size_t index = 0;
        while (len > index) {
            if ((len - index) > 2) {
                uint16_t l = ntohs(*(uint16_t *)(data + index));

                if ((len - index) >= (2 + l)) {
                    RtdeCallback::onReceive(service_, data + index + 2, l);
                } else {
                    break;
                }

                index += 2 + l;
            } else {
                break;
            }
        }
    }

    void onClose(SessionPtr session)
    {
        //        LOGGING(INFO) << "An RTDE session closed ";
    }
};
typedef server<RTDECallback> RtdeAsio;
