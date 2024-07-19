#ifndef __NRF24_DEVICE_H__
#define __NRF24_DEVICE_H__

#include <RadioLib.h>
#include <cstdint>
#include "radio_device.h"

class nRF24Device : public RadioDevice
{
private:
    SPIClass *__radio_spi{nullptr}; // 默认为HSPI
    SPISettings __spi_setting{60000000, MSBFIRST, SPI_MODE0};
    nRF24 *__radio{nullptr};

public:
    /// @brief 构造函数
    /// @param spi_bus spi总线
    /// @param sck spi sck
    /// @param miso spi miso
    /// @param mosi spi mosi
    /// @param ss spi ss/cs
    /// @param irq nrf24 irq
    /// @param rst nrf24 rst
    nRF24Device(uint8_t spi_bus, int8_t sck, int8_t miso, int8_t mosi, int8_t ss, uint32_t irq, uint32_t rst);

    /// @brief 禁止使用默认构造函数
    nRF24Device() = delete;

    virtual ~nRF24Device();

    /// @brief 初始化nrf24
    /// @param freq 载波频率，单位为MHz。默认为2400 MHz。
    /// @param dr 要使用的数据速率，单位为kbps。默认为1000 kbps。
    /// @param pwr 输出功率，单位为dBm。默认值为-12 dBm。
    /// @param addrWidth 地址宽度（以字节为单位）。默认为5个字节。
    /// @return bool
    bool init(int16_t freq = 2400, int16_t dr = 1000, int8_t pwr = -12, uint8_t addrWidth = 5);

    /// @brief 设置包的接收地址
    /// @param addr 下一个数据包应传输到的地址
    /// @return bool
    bool set_transmit_addr(uint8_t *addr);

    /// @brief 设置接收数据的地址
    /// @param pipe_num 应设置地址的管道编号。无论是0还是1，都使用重载方法处理其他管道。
    /// @param addr nRF24将在指定管道上接收新数据包的地址。
    /// @return bool
    bool set_receive_addr(uint8_t pipe_num, uint8_t *addr);

    /// @brief 发送数据
    /// @param message 需要发送的数据
    /// @param size 数据长度
    /// @return bool
    bool send(uint8_t *message, size_t size) override;

    /// @brief 接收数据
    /// @param buffer 接收数据的缓冲区
    /// @param size 接收数据的长度
    /// @return bool
    bool recv(uint8_t *buffer, size_t &size) override;

    int32_t set_frequency(uint32_t frequency) override;

    uint8_t set_power(uint8_t power) override;

    uint32_t set_data_rate(uint32_t rate) override;

    uint8_t set_addr_width(uint8_t addr_width) override;

    bool shutdown() override;

    bool reboot() override;

    void *device() override { return __radio; }
};

#endif
