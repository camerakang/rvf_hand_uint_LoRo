/*
   RadioLib nRF24 Transmit Example

   This example transmits packets using nRF24 2.4 GHz radio_24G module.
   Each packet contains up to 32 bytes of data, in the form of:
    - Arduino String
    - null-terminated char array (C-string)
    - arbitrary binary data (byte array)

   Packet delivery is automatically acknowledged by the receiver.

   For default module settings, see the wiki page
   https://github.com/jgromes/RadioLib/wiki/Default-configuration#nrf24

   For full API reference, see the GitHub Pages
   https://jgromes.github.io/RadioLib/
*/

#ifndef __LoRa_24G_HPP__
#define __LoRa_24G_HPP__

// include the library
#include <RadioLib.h>
#include "bytes_string.hpp"
#include "nrf24_device.h"
nRF24Device __nrf24_a{FSPI, 4, 6, 5, 3, 7, 2};

// enum Mode
// {
//     RECEIVING,
//     SENDING
// };
// volatile Mode currentMode = RECEIVING;
// ICACHE_RAM_ATTR void setreceiveFlag(void);
// volatile bool receivedFlag = false;

// int transmissionState = RADIOLIB_ERR_NONE;

// nRF24 has the following connections:
// #define MISO_24G 6
// #define MOSI_24G 5
// #define CS_24G 3
// #define SCK_24G 4
// #define IRQ_24G 7
// #define CE_24G 2
// SPIClass radio_spi_24G(FSPI);
// SPISettings spiSettings_24G(2000000, MSBFIRST, SPI_MODE0);
// nRF24 radio_24G = new Module(CS_24G, IRQ_24G, CE_24G, RADIOLIB_NC, radio_spi_24G, spiSettings_24G);
// // nRF24 radio_24G = new Module(3, 7, 2);

// or using RadioShield
// https://github.com/jgromes/RadioShield
// nRF24 radio_24G = RadioShield.ModuleA;

void LoRa_24G_init()
{
    byte addr_rvf[] = {0x02, 0x24, 0x46, 0x68, 0x90};
    byte addr_pcie[] = {0x01, 0x23, 0x45, 0x67, 0x89};
    __nrf24_a.init(2402, 1000, 0, 5);
    __nrf24_a.set_transmit_addr(addr_pcie);
    __nrf24_a.set_receive_addr(0, addr_rvf);
}

#endif // __LoRa_24G_HPP__
