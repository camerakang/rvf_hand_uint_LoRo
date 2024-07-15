#include <Arduino.h>
#include "LoRa_900M.hpp"
#include "LoRa_24G.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <queue>

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
  Serial1.begin(115200, SERIAL_8N1, 18, 17);
  Serial1.setRxTimeout(10);
  Serial1.onReceive(onReceive);
  LoRa_24G_init();
  LoRa_900M_init();
  std::thread(LoRa_900M_rx).detach();
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

    // Serial1.write(recv_buf, recv_len);
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

  // if (!dataQueue.empty() && !lengthQueue.empty())
  // {
  //   // 获取并移除长度队列头部的长度值
  //   size_t *length = lengthQueue.front();
  //   if (length != nullptr)
  //   {
  //     // 获取并移除相应长度的数据
  //     uint8_t *data = dataQueue.front();
  //     if (data != nullptr)
  //     {
  //       // 处理接收到的数据
  //       Serial.print(F("Received data: "));
  //       for (size_t i = 0; i < *length; ++i)
  //       {
  //         Serial.print(data[i], HEX);
  //         Serial.print(" ");
  //       }
  //       Serial.println();
  //       // LoRa_24G_tx(data,  *length);
  //       // 移除队列头部的元素
  //       dataQueue.pop();
  //     }
  //     lengthQueue.pop();
  //   }
  // }
  // uint8_t txbuffer[]{0xAA, 0xAB, 0x14, 0x00, 0x08, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x8E, 0x40, 0xC1, 0x3E, 0x31, 0x0A, 0xA3, 0x3E, 0x2D, 0xC8, 0x02, 0x3E, 0x2E, 0x15};
  // uint8_t newBuffer[24 + 1];
  // newBuffer[0] = packetCount;

  // // 将原始数据包复制到新的缓冲区
  // for (uint8_t i = 0; i < 24; ++i)
  // {
  //   newBuffer[i + 1] = txbuffer[i];
  // }

  // // 发送数据包
  // __nrf24_a.send(newBuffer, 24 + 1);

  // // 增加计数编号
  // packetCount++;

  // __nrf24_a.send(txbuffer, 24);
  vTaskDelay(2); // 添加任务延时，防止占用过多CPU时间
  // handle_receive();
}
