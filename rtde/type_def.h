#include <vector>
#include <memory>

///
struct RtdeRecipe
{
    bool to_server;                    ///< 输入/输出
    int chanel;                        ///< 通道
    double frequency;                  ///< 更新频率
    int trigger;                       ///< 触发方式: 0 - 周期; 1 - 变化
    std::vector<std::string> segments; ///< 字段列表
};
