#include "json.hpp"
#include "rtde_recipe.h"

#include <unordered_map>
#include <iostream>

using Json = nlohmann::json;

class RtdeService;

/// 对应每个 RTDE 客户端连接
class RtdeCallback
{
public:
    //    RtdeCallback(LogHandler log_handler);
    RtdeCallback();

    /**
     * 设置 RTDE 输出菜单
     *
     * @param chanel 0~99 通道
     * @param mask 指定的字段
     * @param freq 反馈频率
     */
    void setOutputMask(int chanel, const std::vector<int> &mask, double freq);

    /**
     * 设置 RTDE 输入菜单
     *
     * @param chanel
     * @param mask
     * @param freq
     */
    void setInputMask(int chanel, const std::vector<int> &mask, double freq);

    int update(std::shared_ptr<RtdeService> service,
               std::function<void(char *, size_t)> &&f);

    /**
     * 从客户端接收到了数据 (Json 字符串)，在这里进行处理，调用 service_
     * 的回调函数
     */
    bool onReceive(std::shared_ptr<RtdeService> service, char *buf, size_t len);

    void setRecipe(std::shared_ptr<RtdeService> service, int n, const Json &j);

protected:
    //    LogHandler log_handler_;
    uint64_t last_time = 0;

    std::unordered_map<int, std::vector<int>> output_recipes_;
    std::unordered_map<int, double> output_freqs_;
    std::unordered_map<int, double> output_timess_;

    std::unordered_map<int, std::vector<int>> input_recipes_;
    std::unordered_map<int, double> input_freqs_;
    std::unordered_map<int, double> input_timess_;
};

/// RTDE 服务端
class RtdeService
{
public:
    //    RtdeService(common_interface::AuboControlAPIPtr api);
    RtdeService();

    // 序列化所有的菜单
    void packAll();

    std::vector<uint8_t> pack(int chanel, const std::vector<int> &mask,
                              uint64_t last_time);

    void onReceive(int n, const Json &j);
    void onReceive(const std::string &n, const Json &j);

    Json getInputMap();
    Json getOutputMap();

private:
    bool packRobotMessage(/*RobotInterfacePtr interface, */ Json &js,
                          uint64_t last_time);

private:
    friend class RtdeCallback;

    // RTDE Output 菜单
    std::unordered_map<int, std::vector<int>> output_recipes_;
    std::unordered_map<int, double> output_freqs_;
    std::unordered_map<int, double> output_timess_;

    std::unordered_map<int, std::vector<int>> input_recipes_;

    // RtdeInput 列表: 宏定义展开自动创建 map
    std::map<std::string, RtdeInput> input_recipe_ = {
#define RRII(i, n, ...) { #i, RtdeInput::i },
        RTDE_INPUT_MAP
#undef RRII
    };
    std::map<std::string, RtdeOutput> output_recipe_ = {
#define RRII(i, n, ...) { #i, RtdeOutput::i },
        RTDE_OUTPUT_MAP
#undef RRII
    };

    // 缓存的 RTDE 数据包
    std::unordered_map<int, std::vector<uint8_t>> buffers_;
};
