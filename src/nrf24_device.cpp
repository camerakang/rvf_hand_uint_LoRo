#include "nrf24_device.h"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>

#include "utools.h"

nRF24Device::nRF24Device(uint8_t spi_bus, int8_t sck, int8_t miso, int8_t mosi, int8_t ss, uint32_t irq, uint32_t rst)
{
    __radio_spi = new SPIClass(spi_bus);
    __radio_spi->begin(sck, miso, mosi, ss);
    __radio = new nRF24{new Module{static_cast<uint32_t>(ss), irq, rst, RADIOLIB_NC, *__radio_spi, __spi_setting}};
}

nRF24Device::~nRF24Device()
{
    if (__radio)
    {
        delete __radio;
    }
    if (__radio_spi)
    {
        delete __radio_spi;
    }
}

bool nRF24Device::init(int16_t freq, int16_t dr, int8_t pwr, uint8_t addrWidth)
{
    utools::logger_info("nRF24 device init");
    auto status = __radio->begin(freq, dr, pwr, addrWidth);
    if (status == RADIOLIB_ERR_NONE)
    {
        __radio->setBitRate(dr);
        __radio->setCrcFiltering(false);
        __radio->setAutoAck(false);
        utools::logger_info("nRF24 device init success");
        return true;
    }
    utools::logger_error("nRF24 device init failed. error code:", status);
    return false;
}

bool nRF24Device::set_transmit_addr(uint8_t *addr)
{
    auto status{__radio->setTransmitPipe(addr)};
    utools::logger_info("set transmit addr:", utools::code::to_hex(addr, 5), "status:", status);
    return RADIOLIB_ERR_NONE == status;
}

bool nRF24Device::set_receive_addr(uint8_t pipe_num, uint8_t *addr)
{
    auto status{__radio->setReceivePipe(pipe_num, addr)};
    utools::logger_info("set receive addr:pipe:", utools::code::to_hex(addr, 5), ":", pipe_num, "status:", status);
    return RADIOLIB_ERR_NONE == status;
}

bool nRF24Device::send(uint8_t *message, size_t size)
{
    auto status{__radio->transmit(message, size, 0)};
    if (status == RADIOLIB_ERR_ACK_NOT_RECEIVED)
    {
        // static_cast<nRF24 *>(__radio)->clearIRQ();
        ESP.restart();
    }
    utools::logger_info("send message:", utools::code::to_hex(message, size), "status:", status);
    return RADIOLIB_ERR_NONE == status;
}

bool nRF24Device::recv(uint8_t *buffer, size_t &size)
{
    size = __radio->getPacketLength(); // 最大数据包长度
    int status = __radio->receive(buffer, size);

    if (status == RADIOLIB_ERR_NONE)
    {
        return true;
    }
    return false;
}

int32_t nRF24Device::set_frequency(uint32_t frequency)
{
    return __radio->setFrequency(frequency / 1000000);
}

uint8_t nRF24Device::set_power(uint8_t power)
{
    return 0;
}

uint32_t nRF24Device::set_data_rate(uint32_t rate)
{
    // return __radio->setDataRate(rate);
    return 0;
}

uint8_t nRF24Device::set_addr_width(uint8_t addr_width)
{
    return 0;
}

bool nRF24Device::shutdown()
{
    return RADIOLIB_ERR_NONE == __radio->sleep();
}

bool nRF24Device::reboot()
{
    return true;
}