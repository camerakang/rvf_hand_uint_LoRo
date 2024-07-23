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

utools::collab::SyncQueue<std::vector<uint8_t>, 3> rx_queue;

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
    rx_queue.emplace(buffer, buffer + bytesRead); // 预先确认队列中有足够的空间存储数据
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

void loop()
{
  auto rx_data = rx_queue.pop_wait();
  __nrf24_a.send(rx_data.data(), rx_data.size());
}
