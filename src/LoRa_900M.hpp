/*
  RadioLib SX126x Blocking Transmit Example

  This example transmits packets using SX1262 LoRa radio_900M module.
  Each packet contains up to 256 bytes of data, in the form of:
  - Arduino String
  - null-terminated char array (C-string)
  - arbitrary binary data (byte array)

  Other modules from SX126x family can also be used.

  Using blocking transmit is not recommended, as it will lead
  to inefficient use of processor time!
  Instead, interrupt transmit is recommended.

  For default module settings, see the wiki page
  https://github.com/jgromes/RadioLib/wiki/Default-configuration#sx126x---lora-modem

  For full API reference, see the GitHub Pages
  https://jgromes.github.io/RadioLib/
*/

// include the library
#include <RadioLib.h>
#include <queue>
#include <vector>
#include "bytes_string.hpp"
#include "nrf24_device.h"
std::queue<std::vector<uint8_t>> txBufferQueue;
uint8_t parseProtocol(uint8_t *data, size_t length);
volatile bool receivedFlag_900 = false;

extern "C" void setFlag(void)
{
    // we got a packet, set the flag
    receivedFlag_900 = true;
}
void LoRa_900M_init();
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
SX1262 radio_900M = new Module(NSS_900, IRQ_900, RST_900, BUSY_900, radio_spi_900M, spiSettings_900M);

// or using RadioShield
// https://github.com/jgromes/RadioShield
// SX1262 radio_900M = RadioShield.ModuleA;

// or using CubeCell
// SX1262 radio_900M = new Module(RADIOLIB_BUILTIN_MODULE);

void LoRa_900M_init()
{
    radio_spi_900M.begin(SCK_900, MISO_900, MOSI_900, NSS_900);
    // 使用温度补偿晶振
    radio_900M.XTAL = true;
    // initialize SX1262 with default settings
    Serial.print(F("[SX1262] Initializing ... "));
    // int state = radio_900M.begin(915.0, 125.0, 9, 7, 0x12, 10, 8, 1.6, false);
    int state = radio_900M.beginFSK(915.0,300.0,5.0,156.2);
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {

        while (true)
        {
            Serial.print(F("9XX failed, code "));
            Serial.println(state);
            delay(1000);
        }
    }
    radio_900M.setPacketReceivedAction(setFlag);

    // some modules have an external RF switch
    // controlled via two pins (RX enable, TX enable)
    // to enable automatic control of the switch,
    // call the following method
    // RX enable:   4
    // TX enable:   5
    /*
      radio_900M.setRfSwitchPins(RX_900, TX_900);
    */
    radio_900M.setRfSwitchPins(RX_900, TX_900);
    Serial.print(F("[SX1262] Starting to listen ... "));
    state = radio_900M.startReceive();
    if (state == RADIOLIB_ERR_NONE)
    {
        Serial.println(F("success!"));
    }
    else
    {
        Serial.print(F("failed, code "));
        Serial.println(state);
        while (true)
            ;
    }
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
void LoRa_900M_rx()
{

    while (1)
    {
        uint8_t buffer[128];

        // check if the flag is set
        if (receivedFlag_900)
        {
            // reset flag
            receivedFlag_900 = false;

            // you can read received data as an Arduino String
            size_t len = radio_900M.getPacketLength();
            int state = radio_900M.readData(buffer, 0);

            if (state == RADIOLIB_ERR_NONE)
            {
                Serial.print("receive form 9xx:");
                Serial.println(to_hex_str(buffer, len).c_str());
                if (parseProtocol(buffer, len))
                {
                    Serial.print("change channle suss");
                    ESP.restart();
                }
                else
                {
                    // 预先确认队列中有足够的空间存储数据
                    txBufferQueue.emplace(buffer, buffer + len); // 将数据放入队列
                }
            }
            else if (state == RADIOLIB_ERR_CRC_MISMATCH)
            {
                // packet was received, but is malformed
                Serial.println(F("CRC error!"));
            }
            else
            {
                // some other error occurred
                Serial.print(F("failed, code "));
                Serial.println(state);
            }
        }
        if (!txBufferQueue.empty())
        {
            auto data = txBufferQueue.front();
            txBufferQueue.pop();
            Serial1.write(data.data(), data.size());
            Serial.print("send to serial1:");
            Serial.println(to_hex_str(data.data(), data.size()).c_str());
        }
        // uint8_t test_buffer[] = {0xaa, 0xab, 0x1a, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xd4, 0xbb, 0xeb, 0x43, 0xe9, 0x88, 0x0f, 0x43, 0x00, 0x00, 0xa0, 0x41, 0x00, 0x00, 0x80, 0x3f, 0x01, 0x01, 0x35, 0x06};
        // // Serial1.write(data.data(), data.size());
        // Serial1.write(test_buffer, sizeof(test_buffer));
        delay(10);
    }
}

uint8_t parseProtocol(uint8_t *data, size_t length)
{
    // 检查数据长度
    if (length != 11)
    {
        Serial.println("Invalid data length");
        return 0;
    }

    // 检查前7个字节和最后3个字节是否符合特定模式
    if (memcmp(data, (const uint8_t[]){0xc0, 0x00, 0x08, 0x00, 0x00, 0xe7, 0x80}, 7) == 0 && memcmp(data + 8, (const uint8_t[]){0x00, 0x00, 0x00}, 3) == 0)
    {
        /// TODO: 使用原本频率通知电机和电脑板 等待返回以后修改频率
        int state;
        Serial.print("data:");
        Serial.println(data[7], HEX);
        state = __nrf24_a.sleep();
        Serial.print("state:");
        Serial.println(state);
        switch (data[7])
        {
        case 0x00:
            state = __nrf24_a.setFrequency(2402);
            break;
        case 0x05:
            state = __nrf24_a.setFrequency(2409);
            break;
        case 0x0a:
            state = __nrf24_a.setFrequency(2416);
            break;
        case 0x0f:
            state = __nrf24_a.setFrequency(2423);
            break;
        case 0x14:
            state = __nrf24_a.setFrequency(2430);
            break;
        case 0x19:
            state = __nrf24_a.setFrequency(2437);
            break;
        case 0x1e:
            state = __nrf24_a.setFrequency(2444);
            break;
        case 0x23:
            state = __nrf24_a.setFrequency(2451);
            break;
        case 0x28:
            state = __nrf24_a.setFrequency(2458);
            break;
        case 0x2d:
            state = __nrf24_a.setFrequency(2465);
            break;
        case 0x32:
            state = __nrf24_a.setFrequency(2472);
            break;
        case 0x37:
            state = __nrf24_a.setFrequency(2479);
            break;
        case 0x3c:
            state = __nrf24_a.setFrequency(2486);
            break;
        case 0x41:
            state = __nrf24_a.setFrequency(2493);
            break;
        case 0x46:
            state = __nrf24_a.setFrequency(2500);
            break;
        case 0x4b:
            state = __nrf24_a.setFrequency(2507);
            break;

        default:
            break;
        }
        state = __nrf24_a.standby();
        if (state == RADIOLIB_ERR_NONE)
        {
            return 1;
        }
    }
    else
    {
        // 数据不匹配
        Serial.println("Data does not match the expected pattern.");
        return 0;
    }
}