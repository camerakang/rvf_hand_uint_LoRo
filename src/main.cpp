#include <Arduino.h>
#include "LoRa_900M.hpp"
#include "LoRa_24G.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <queue>

#include "utools.h"

#define BUFFER_SIZE 10
std::unique_ptr<unsigned char[]> rxBuffer;
volatile size_t rxLength = 0;
volatile bool dataAvailable = false;
TaskHandle_t Task1Handle = NULL;

#include "ring_queue.hpp"

// 定义队列长度
#define QUEUE_CAPACITY 128
#define LENGTH_QUEUE_CAPACITY 32

// 创建环形队列对象
// RingQueue<uint8_t> dataQueue(QUEUE_CAPACITY);
// RingQueue<size_t> lengthQueue(LENGTH_QUEUE_CAPACITY);

std::queue<std::vector<uint8_t>> rxBufferQueue;
uint8_t parseProtocol(const uint8_t *data, size_t length);
void handle_receive();
// 串口1数据接收中断处理函数
void IRAM_ATTR onReceive()
{
  size_t len = Serial1.available();
  uint8_t buffer[128];
  if (len > 0)
  {
    size_t bytesRead = Serial1.read(buffer, len);

    // 预先确认队列中有足够的空间存储数据
    // Serial.print("uart Received: ");
    // Serial.println(to_hex_str(buffer, len).c_str());
    rxBufferQueue.emplace(buffer, buffer + bytesRead);
  }
}

void setup()
{
  Serial.begin(115200);

  utools::logger::set_log_fun([](const char *msg) -> void
                              { Serial.print(msg); });
  utools::logger::set_log_levels({utools::logger::level::INFO,
                                  utools::logger::level::TRACE,
                                  utools::logger::level::DEBUG,
                                  utools::logger::level::WARN,
                                  utools::logger::level::ERROR,
                                  utools::logger::level::FATAL});
  utools::logger_trace("utools configured.");

  Serial1.begin(115200, SERIAL_8N1, 18, 17);
  Serial1.setRxTimeout(10);
  Serial1.onReceive(onReceive);

  // 初始化 LoRa_24G
  LoRa_24G_init();
  // 初始化 LoRa_900M
  LoRa_900M_init();
}

uint64_t receive_times = 0;
void handle_receive()
{
  uint8_t *recv_buf{new uint8_t[128]{0}};
  size_t recv_len{0};

  if (__nrf24_a.recv(recv_buf, recv_len))
  {
    // 如果接收成功，则处理数据
    Serial.println(receive_times++);
    Serial.print("\t");
    Serial.println(to_hex_str(recv_buf, recv_len).c_str());
  }
  delete recv_buf;
}
uint8_t packetCount = 0;
void loop()
{
  // Serial.println("loop");
  // if (rxLength > 0)
  //   LoRa_24G_tx(rxBuffer.get(), rxLength);
  // 检查数据队列和长度队列是否为空
  if (!rxBufferQueue.empty())
  {
    auto data = rxBufferQueue.front();
    rxBufferQueue.pop();
    __nrf24_a.send(data.data(), data.size());
    // uint8_t test_buffer[] = {0xaa, 0xab, 0x1a, 0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xd4, 0xbb, 0xeb, 0x43, 0xe9, 0x88, 0x0f, 0x43, 0x00, 0x00, 0xa0, 0x41, 0x00, 0x00, 0x80, 0x3f, 0x01, 0x01, 0x35, 0x06};
    // __nrf24_a.send(test_buffer, sizeof(test_buffer));
  }

  vTaskDelay(2); // 添加任务延时，防止占用过多CPU时间
}

// // include the library
// #include <Arduino.h>
// #include <RadioLib.h>

// // nRF24 has the following connections:
// #define MISO_24G 6
// #define MOSI_24G 5
// #define CS_24G 3
// #define SCK_24G 4
// #define IRQ_24G 7
// #define CE_24G 2
// SPIClass radio_spi_24G(FSPI);
// SPISettings spiSettings_24G(60000000, MSBFIRST, SPI_MODE0);
// nRF24 *radio;

// void setup()
// {
//   delay(1000);
//   Serial.begin(115200);

//   // initialize nRF24 with default settings
//   Serial.print(F("[nRF24] Initializing ... "));

//   delay(1000);

//   radio = new nRF24(new Module(CS_24G, IRQ_24G, CE_24G, RADIOLIB_NC, radio_spi_24G, spiSettings_24G));
//   int state = radio->begin();
//   if (state == RADIOLIB_ERR_NONE)
//   {
//     Serial.println(F("success!"));
//   }
//   else
//   {
//     Serial.print(F("failed, code "));
//     Serial.println(state);
//     while (true)
//       ;
//   } // set transmit address
//   // NOTE: address width in bytes MUST be equal to the
//   //       width set in begin() or setAddressWidth()
//   //       methods (5 by default)
//   byte addr[] = {0x01, 0x23, 0x45, 0x67, 0x89};
//   Serial.print(F("[nRF24] Setting transmit pipe ... "));
//   state = radio->setTransmitPipe(addr);
//   if (state == RADIOLIB_ERR_NONE)
//   {
//     Serial.println(F("success!"));
//   }
//   else
//   {
//     Serial.print(F("failed, code "));
//     Serial.println(state);
//     while (true)
//       ;
//   }
// }

// // counter to keep track of transmitted packets
// int count = 0;

// void loop()
// {
//   Serial.print(F("[nRF24] Transmitting packet ... "));

//   // you can transmit C-string or Arduino string up to
//   // 32 characters long
//   String str = "Hello World! #" + String(count++);
//   int state = radio->transmit(str);

//   if (state == RADIOLIB_ERR_NONE)
//   {
//     // the packet was successfully transmitted
//     Serial.println(F("success!"));
//   }
//   else if (state == RADIOLIB_ERR_PACKET_TOO_LONG)
//   {
//     // the supplied packet was longer than 32 bytes
//     Serial.println(F("too long!"));
//   }
//   else if (state == RADIOLIB_ERR_ACK_NOT_RECEIVED)
//   {
//     // acknowledge from destination module
//     // was not received within 15 retries
//     Serial.println(F("ACK not received!"));
//   }
//   else if (state == RADIOLIB_ERR_TX_TIMEOUT)
//   {
//     // timed out while transmitting
//     Serial.println(F("timeout!"));
//   }
//   else
//   {
//     // some other error occurred
//     Serial.print(F("failed, code "));
//     Serial.println(state);
//   }

//   // wait for a second before transmitting again
//   delay(1000);
// }
