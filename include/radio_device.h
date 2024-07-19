#ifndef __RADIO_DEVICE_H__
#define __RADIO_DEVICE_H__

#include <cstdint>
#include <cstddef>
#include <vector>

class RadioDevice
{
public:
    RadioDevice() = default;
    virtual ~RadioDevice() = default;

    /// @brief 初始化设备
    /// @return 是否成功初始化
    virtual bool init() { return true; }

    /// @brief 发送数据
    /// @param message 需要发送的数据
    /// @param size 数据长度
    /// @return bool
    virtual bool send(uint8_t *message, size_t size) = 0;

    virtual bool send(const std::vector<uint8_t> &message)
    {
        return send((uint8_t *)message.data(), message.size());
    }

    /// @brief 接收数据
    /// @param buffer 接收数据的缓冲区
    /// @param size 接收数据的长度
    /// @return bool
    virtual bool recv(uint8_t *buffer, size_t &size) = 0;

    /// @brief 设置设备的工作频率
    /// @param frequency_hz 目前工作频率，单位为Hz
    /// @return 返回设置后真实的频率，单位为Hz，0表示失败
    virtual int32_t set_frequency(uint32_t frequency_hz) = 0;

    /// @brief 设置设备的功率
    /// @param power 输出功率，单位为dBm。默认值为-12 dBm。
    /// @return 返回设置后真实的功率，单位为dBm，0表示失败
    virtual uint8_t set_power(uint8_t power) = 0;

    /// @brief 设置数据速率
    /// @param rate 数据速率，单位为kbps。默认值为250 kbps。
    /// @return 返回设置后真实的数据速率，单位为kbps，0表示失败
    virtual uint32_t set_data_rate(uint32_t rate) = 0;

    /// @brief 设置地址宽度
    /// @param addr_width 地址宽度（以字节为单位）。默认为5个字节。
    /// @return 返回设置后真实的地址宽度，单位为字节，0表示失败
    virtual uint8_t set_addr_width(uint8_t addr_width) = 0;

    /// @brief 关闭设备
    /// @return 是否成功关闭
    virtual bool shutdown() = 0;

    // virtual bool boot() = 0;

    /// @brief 重启设备
    /// @return 是否成功重启
    virtual bool reboot() = 0;

    /// @brief 获取设备的操作管理权
    /// @return 设备对象
    virtual void *device() = 0;
};

#endif // __RADIO_DEVICE_H__