#include "rtde_service.h"
#include "type_def.h"

RtdeCallback::RtdeCallback()
{
}

void RtdeCallback::setOutputMask(int chanel, const std::vector<int> &mask,
                                 double freq)
{
    if (output_recipes_.find(chanel) == output_recipes_.end()) {
        output_recipes_[chanel] = mask;

        if (freq < 1e-6) {
            freq = 200.;
        }
        output_freqs_[chanel] = freq;
    }
}

void RtdeCallback::setInputMask(int chanel, const std::vector<int> &mask,
                                double freq)
{
    if (input_recipes_.find(chanel) == input_recipes_.end()) {
        input_recipes_[chanel] = mask;

        if (freq < 1e-6) {
            freq = 200.;
        }
        input_freqs_[chanel] = freq;
    }
}

int RtdeCallback::update(std::shared_ptr<RtdeService> service,
                         std::function<void(char *, size_t)> &&f)
{
    //    try {
    //        std::vector<uint8_t> buf;

    //        for (auto [chanel, mask] : output_recipes_) {
    //            if (output_timess_.find(chanel) != output_timess_.end()) {
    //                if (output_timess_[chanel] <= 1e-6) {
    //                    /*LOGGING(WARNING)
    //                            << "Send RTDE package: " <<
    //                            sub_masks_time_[it] <<
    //                           ", "
    //                            << 1. / sub_masks_freq_[it];*/
    //                    output_timess_[chanel] += 1. / output_freqs_[chanel];
    //                    // EASY_BLOCK("AUBO_COMM:RTDE_ASIO_PACK"); buf =
    //                    service->pack(chanel, mask, last_time);
    //                    //                    EASY_END_BLOCK;
    //                    //                    last_time =
    //                    service->currentTime();

    //                    // 发送出去: 需要有数据
    //                    if (f && buf.size()) {
    //                        // EASY_BLOCK("AUBO_COMM:RTDE_ASIO_SEND"); f((char
    //                        *)buf.data(), buf.size());
    //                        //                        EASY_END_BLOCK;
    //                    }
    //                }
    //                output_timess_[chanel] -= 0.005;
    //            } else {
    //                output_timess_[chanel] = 1. / output_freqs_[chanel];
    //            }
    //        }
    //    } catch (...) {
    //    }

    return 0;
}

bool RtdeCallback::onReceive(std::shared_ptr<RtdeService> service, char *buf,
                             size_t len)
{
    Json j;
    bool parsed = false;

    // 支持两种解析方式(CBOR/JSON)
    try {
        j = Json::from_cbor(buf, buf + len);
        parsed = true;
    } catch (Json::parse_error e) {
        //        LOGGING(DEBUG) << "Parse as bson FAILED:" << e.what();
    }

    try {
        if (!parsed) {
            j = Json::parse(buf, buf + len);
            parsed = true;
        }
    } catch (Json::parse_error e) {
        //        LOGGING(DEBUG) << "Parse as json FAILED:" << e.what();
    }
    if (!parsed) {
        //        LOGGING(DEBUG) << "Json parsed FAILED: len " << len << " " <<
        //        buf;
        return false;
    }

    //    LOGGING(DEBUG) << "onReceive(" << len << ") " << j.dump();

    // 提取topic和param
    if (!j.is_array()) {
        // 要求JSON为一个数组
        //        LOGGING(DEBUG) << "RTDE recieve not a array";
        return false;
    }

    auto topic = j[0];

    try {
        if (topic.is_number_unsigned()) {
            int num = topic.get<int>();
            if ((num >= 0) && (num < 100)) { // 0~99 为用户设置通道
                // 组合
                int index = 1;
                for (auto i : input_recipes_[num]) {
                    service->onReceive(i, j[index]);
                    setRecipe(service, i, j[index]);
                    index++;
                }
            } else if (num >= 100) {
                // 单一
                service->onReceive(num, j[1]);
                setRecipe(service, num, j[1]);
            }
        } else if (topic.is_string()) {
            // 单一
            service->onReceive(topic.get<std::string>(), j[1]);
            setRecipe(service, service->input_recipe_[topic.get<std::string>()],
                      j[1]);
        }
    } catch (nlohmann::detail::type_error e) {
        //        LOGGING(DEBUG) << "Parse topic " << topic
        //                       << " as vector int: " << e.what() << std
        //                       ::endl;
    }

    return true;
}

void RtdeCallback::setRecipe(std::shared_ptr<RtdeService> service, int n,
                             const Json &j)
{
    //    if (n == RtdeInput::set_recipe) {
    //        //        LOGGING(INFO) << "RTDE input add_output_mask";
    //        std::vector<int> tmp;
    //        //        LOGGING(INFO) << "j: " << j.dump();

    //        RtdeRecipe rtde_output_map;
    //        j.get_to(rtde_output_map);

    //        //        LOGGING(INFO) << "freq: " << rtde_output_map.frequency;

    //        for (auto it : rtde_output_map.segments) {
    //            if (service->output_recipe_.find(it) !=
    //                service->output_recipe_.end()) {
    //                int idx = service->output_recipe_.find(it)->second;
    //                if (idx < 0) {
    //                    // 无法查找到
    //                    return;
    //                }
    //                //                LOGGING(INFO) << "recp: " << it << " id:
    //                " <<
    //                //                idx;

    //                tmp.push_back(idx);
    //            } else {
    //                //                LOGGING(INFO) << "Cannot find " << it <<
    //                "
    //                //                with output recipe";
    //            }
    //        }

    //        if (rtde_output_map.to_server) {
    //            setInputMask(rtde_output_map.chanel, tmp,
    //                         rtde_output_map.frequency);
    //        } else {
    //            setOutputMask(rtde_output_map.chanel, tmp,
    //                          rtde_output_map.frequency);
    //        }
    //    }
}

RtdeService::RtdeService()
{
}

void RtdeService::packAll()
{
    //    for (auto [chanel, mask] : output_recipes_) {
    //        Json msg;
    //        int shift = 0;

    //        // msg首位存储包含的recipe_id
    //        msg[shift++] = chanel;

    //        for (auto m : mask) {
    //            // LOGGING(DEBUG) << "pack: " << index;
    //            //            parseFromServer(m, interface_, msg[shift++]);
    //        }
    //        // LOGGING(DEBUG) << "pack: " << msg.dump();
    //        auto cbor = Json::to_cbor(msg);
    //        uint16_t len = cbor.size();

    //        std::vector<uint8_t> buf;
    //        buf.reserve(2 + len);
    //        buf.push_back((len >> 8) & 0xFF);
    //        buf.push_back((len >> 0) & 0xFF);
    //        std::copy(cbor.begin(), cbor.end(), std::back_inserter(buf));

    //        buffers_[chanel] = buf;
    //    }
}

std::vector<uint8_t> RtdeService::pack(int chanel, const std::vector<int> &mask,
                                       uint64_t last_time)
{
    // 实时消息特殊处理, 需要根据不同的用户做不同的处理
    if (chanel == RtdeOutput::R1_message) {
        Json msg;
        msg[0] = chanel;
        //        auto robot =
        //            interface_->getRobotInterface(interface_->getRobotNames().front());
        //        if (!robot) {
        //            // 机器人不存在
        //            return {};
        //        }

        //        if (packRobotMessage(robot, msg[1], last_time))
        {
            auto cbor = Json::to_cbor(msg);
            uint16_t len = cbor.size();
            //            LOGGING(INFO) << "packRobotMessage cbor: " << len << "
            //            "
            //            << msg.dump();

            std::vector<uint8_t> buf;
            buf.reserve(2 + len);
            buf.push_back((len >> 8) & 0xFF);
            buf.push_back((len >> 0) & 0xFF);
            std::copy(cbor.begin(), cbor.end(), std::back_inserter(buf));

            return buf;
        }

        // 无需要上传的消息
        return {};
    } else {
        // LOGGING(DEBUG) << "pack: " << chanel;
        Json msg;
        int shift = 0;

        bool ok = true;
        // msg首位存储包含的recipe_id
        msg[shift++] = chanel;

        //        EASY_BLOCK("AUBO_COMM:TO_JSON");
        for (auto m : mask) {
            //            ok = parseFromServer(m, interface_, msg[shift++]) ==
            //            0;
            if (!ok) {
                // LOGGING(WARNING) << "pack: " << m << " failed";
                break;
            }
        }
        //        EASY_END_BLOCK;
        // LOGGING(DEBUG) << "pack: " << msg.dump();
        if (ok) {
            /*EASY_BLOCK("AUBO_COMM:TO_CBOR");
            auto cbor = Json::to_cbor(msg);
            uint16_t len = cbor.size();*/
            auto json_str = msg.dump();
            uint16_t len = json_str.size();

            std::vector<uint8_t> buf;
            buf.reserve(2 + len);
            buf.push_back((len >> 8) & 0xFF);
            buf.push_back((len >> 0) & 0xFF);
            std::copy(json_str.begin(), json_str.end(),
                      std::back_inserter(buf));
            //            EASY_END_BLOCK;

            return buf;
        } else {
            return {};
        }
    }
}

void RtdeService::onReceive(int n, const Json &j)
{
}

void RtdeService::onReceive(const std::string &n, const Json &j)
{
}

Json RtdeService::getInputMap()
{
}

Json RtdeService::getOutputMap()
{
}

bool RtdeService::packRobotMessage(Json &js, uint64_t last_time)
{
}
