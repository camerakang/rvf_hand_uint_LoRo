#include "nrf24_device.h"

#include <condition_variable>
#include <mutex>
#include <thread>
#include <atomic>
#include "bytes_string.hpp"
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
    auto status = __radio->begin(freq, dr, pwr, addrWidth);

    if (status == RADIOLIB_ERR_NONE)
    {
        __radio->setBitRate(1000);
        __radio->setCrcFiltering(false);
        __radio->setAutoAck(false);
        __radio->setFrequency(freq);
        return true;
    }
    else
    {

        while (true)
        {
            Serial.print(F("nf24 failed, code "));
            Serial.println(status);
            delay(1000);
        }
    }
    return false;
}

bool nRF24Device::set_transmit_addr(uint8_t *addr)
{
    return __radio->setTransmitPipe(addr) == RADIOLIB_ERR_NONE;
}

bool nRF24Device::set_receive_addr(uint8_t pipe_num, uint8_t *addr)
{
    return __radio->setReceivePipe(pipe_num, addr) == RADIOLIB_ERR_NONE;
}
uint64_t sendtimes = 0;
bool nRF24Device::send(uint8_t *message, size_t size)
{

    auto status{__radio->transmit(message, size, 0)};
    if (RADIOLIB_ERR_NONE == status)
    {
        Serial.print("send--->>> ");
        Serial.print(sendtimes++);
        Serial.print("\t");
        Serial.println(to_hex_str(message, size, true).c_str());
    }
    else
    {
        Serial.print(F("send failed, code "));
        Serial.println(status);
        ESP.restart();
    }
    return RADIOLIB_ERR_NONE == status;
}

bool nRF24Device::recv(uint8_t *buffer, size_t &size)
{
    size = __radio->getPacketLength(); // 最大数据包长度
    int status = __radio->receive(buffer, size);

    if (status == RADIOLIB_ERR_NONE)
    {
        Serial.print("recv--->>> ");
        Serial.println(to_hex_str(buffer, size, true).c_str());
        return true;
    }
    return false;
}

nRF24 &nRF24Device::device() const
{
    return *__radio;
}

int16_t nRF24Device::setFrequency(float freq)
{
    return __radio->setFrequency(freq);
}
int16_t nRF24Device::sleep()
{
    return __radio->sleep();
}

int16_t nRF24Device::standby()
{
    return __radio->standby();
}
