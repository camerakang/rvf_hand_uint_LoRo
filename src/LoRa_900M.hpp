#include <RadioLib.h>
#include <queue>
#include <vector>
#include <thread>

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "bytes_string.hpp"
#include "nrf24_device.h"
#include "utools.h"
#include "LoRa_24G.hpp"

auto sync_partner_900M{utools::collab::freertos::make_sync_partner<false, true>()};

ICACHE_RAM_ATTR void setFlag(void)
{
    sync_partner_900M.notify();
}

static uint8_t parseProtocol(uint8_t *data, size_t length);
void LoRa_900M_init();
void LoRa_900M_rx(void *pvParameters);

// SX1262 has the following connections:
// NSS pin:   10
// DIO1 pin:  2
// NRST pin:  3
// BUSY pin:  9
#define MISO_900 8
#define MOSI_900 9
#define NSS_900 10
#define SCK_900 11

#define IRQ_900 12
#define RST_900 15
#define TX_900 -1
#define RX_900 14
#define BUSY_900 13

SPIClass radio_spi_900M(HSPI);
SPISettings spiSettings_900M(2000000, MSBFIRST, SPI_MODE0);
Module *modeuld_900M = new Module(NSS_900, IRQ_900, RST_900, BUSY_900, radio_spi_900M, spiSettings_900M);
SX1262 radio_900M = SX1262(modeuld_900M);

void LoRa_900M_init()
{
    radio_spi_900M.begin(SCK_900, MISO_900, MOSI_900, NSS_900);
    // 使用温度补偿晶振
    radio_900M.XTAL = true;
    // initialize SX1262 with default settings
    utools::logger_info("Initializing SX1262");
    // int state = radio_900M.begin(915.0, 125.0, 9, 7, 0x12, 10, 8, 1.6, false);
    int state = radio_900M.beginFSK(915.0, 30.0, 5.0, 156.2, 22);
    if (state == RADIOLIB_ERR_NONE)
    {
        utools::logger_info("SX1262 Module init success!");
    }
    else
    {
        utools::logger_error("SX1262 Module init failed, code:", state);
        return;
    }

    // radio_900M.setPacketReceivedAction(setFlag);
    radio_900M.setDio1Action(setFlag);

    // some modules have an external RF switch
    // controlled via two pins (RX enable, TX enable)
    // to enable automatic control of the switch,
    // call the following method
    // RX enable:   4
    // TX enable:   5
    radio_900M.setRfSwitchPins(RX_900, TX_900);
    utools::logger_info("SX1262 Starting to listen ... ");
    state = radio_900M.startReceive();
    if (state == RADIOLIB_ERR_NONE)
    {
        utools::logger_info("SX1262 Start to listen success!");
    }
    else
    {
        utools::logger_error("SX1262 Start to listen failed, code:", state);
    }

    xTaskCreate(LoRa_900M_rx, "LoRa_900M_rx", 1024 * 10, NULL, 1, NULL); // 启动接收任务
}

#if defined(ESP8266) || defined(ESP32)
ICACHE_RAM_ATTR
#endif

// counter to keep track of transmitted packets
int count_900M = 0;

void LoRa_900M_tx()
{
    while (1)
    {
        Serial.print(F("[SX1262] Transmitting packet ... "));

        // you can transmit C-string or Arduino string up to
        // 256 characters long
        String str = "Hello World! #" + String(count_900M++);
        int state = radio_900M.transmit(str);

        // you can also transmit byte array up to 256 bytes long
        /*
          byte byteArr[] = {0x01, 0x23, 0x45, 0x56, 0x78, 0xAB, 0xCD, 0xEF};
          int state = radio_900M.transmit(byteArr, 8);
        */

        if (state == RADIOLIB_ERR_NONE)
        {
            // the packet was successfully transmitted
            Serial.println(F("success!"));

            // print measured data rate
            Serial.print(F("[SX1262] Datarate:\t"));
            Serial.print(radio_900M.getDataRate());
            Serial.println(F(" bps"));
        }
        else if (state == RADIOLIB_ERR_PACKET_TOO_LONG)
        {
            // the supplied packet was longer than 256 bytes
            Serial.println(F("too long!"));
        }
        else if (state == RADIOLIB_ERR_TX_TIMEOUT)
        {
            // timeout occured while transmitting packet
            Serial.println(F("timeout!"));
        }
        else
        {
            // some other error occurred
            Serial.print(F("send failed, code "));
            Serial.println(state);
        }

        // wait for a second before transmitting again
        delay(1000);
    }
}

void LoRa_900M_rx(void *pvParameters)
{
    uint8_t buffer[128];

    while (1)
    {
        sync_partner_900M.wait(); // 等待同步信号，会在此处理一直等待
        size_t len = radio_900M.getPacketLength();
        int state = radio_900M.readData(buffer, 0);
        if (state == RADIOLIB_ERR_NONE)
        {
            // utools::logger_info("recieve form 9xx:", utools::code::to_hex(buffer, len));
            if (parseProtocol(buffer, len))
            {
                utools::logger_info("change channle suss");
                ESP.restart();
            }
            else
            {
                Serial1.write(buffer, len);
                // utools::logger_trace("send to serial1:", utools::code::to_hex(buffer, len));
            }
        }
        else if (state == RADIOLIB_ERR_CRC_MISMATCH)
        {
            utools::logger_error("crc error, data:", utools::code::to_hex(buffer, len));
        }
        else
        {
            utools::logger_error("recieve failed code:", state);
        }
    }
}

uint8_t parseProtocol(uint8_t *data, size_t length)
{
    // 检查数据长度
    if (length != 11)
    {
        // Serial.println("Invalid data length");
        return 0;
    }

    // 检查前7个字节和最后3个字节是否符合特定模式
    if (memcmp(data, (const uint8_t[]){0xc0, 0x00, 0x08, 0x00, 0x00, 0xe7, 0x80}, 7) == 0 && memcmp(data + 8, (const uint8_t[]){0x00, 0x00, 0x00}, 3) == 0)
    {
        /// TODO: 使用原本频率通知电机和电脑板 等待返回以后修改频率
        int state;
        Serial.print("data:");
        Serial.println(data[7], HEX);
        auto nrf24_dev = static_cast<nRF24 *>(__nrf24_a.device());
        state = nrf24_dev->sleep();
        Serial.print("state:");
        Serial.println(state);
        switch (data[7])
        {
        case 0x00:
            state = nrf24_dev->setFrequency(2402);
            break;
        case 0x05:
            state = nrf24_dev->setFrequency(2409);
            break;
        case 0x0a:
            state = nrf24_dev->setFrequency(2416);
            break;
        case 0x0f:
            state = nrf24_dev->setFrequency(2423);
            break;
        case 0x14:
            state = nrf24_dev->setFrequency(2430);
            break;
        case 0x19:
            state = nrf24_dev->setFrequency(2437);
            break;
        case 0x1e:
            state = nrf24_dev->setFrequency(2444);
            break;
        case 0x23:
            state = nrf24_dev->setFrequency(2451);
            break;
        case 0x28:
            state = nrf24_dev->setFrequency(2458);
            break;
        case 0x2d:
            state = nrf24_dev->setFrequency(2465);
            break;
        case 0x32:
            state = nrf24_dev->setFrequency(2472);
            break;
        case 0x37:
            state = nrf24_dev->setFrequency(2479);
            break;
        case 0x3c:
            state = nrf24_dev->setFrequency(2486);
            break;
        case 0x41:
            state = nrf24_dev->setFrequency(2493);
            break;
        case 0x46:
            state = nrf24_dev->setFrequency(2500);
            break;
        case 0x4b:
            state = nrf24_dev->setFrequency(2507);
            break;

        default:
            break;
        }
        state = nrf24_dev->standby();
        if (state == RADIOLIB_ERR_NONE)
        {
            return 1;
        }
    }
    // 数据不匹配
    Serial.println("Data does not match the expected pattern.");
    return 0;
}