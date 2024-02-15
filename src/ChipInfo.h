/*

  ChipInfo.h


*/

#ifndef __CHIP_INFO_H__
#define __CHIP_INFO_H__

#include <Arduino.h>
#include <core_version.h> // For ARDUINO_ESP32_RELEASE
#include "Config.h"

class ChipInfo
{
private:
public:
  static void print();
};

void ChipInfo::print()
{
  //--- Display ESP32 Chip Info
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  Serial.printf("\n\n---------------Infos ESP32----------------------\n");
  Serial.printf("Arduino Release : %s\n", ARDUINO_ESP32_RELEASE);
  Serial.printf("Chip Revision : %d\n", chip_info.revision);
  Serial.printf("SDK : %s\n", ESP.getSdkVersion());
  Serial.printf("CPU freq : %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("CPU cores : %d\n", chip_info.cores);
  Serial.printf("Flash size :  %d MB ", spi_flash_get_chip_size() / (1024 * 1024));
  Serial.println(((chip_info.features & CHIP_FEATURE_EMB_FLASH) != 0) ? "(embeded)" : "(external)");
  Serial.printf("APB CLOCK : %d MHz\n", APB_CLK_FREQ / (1000 * 1000));
  Serial.printf("Free RAM : %ld bytes\n", (long)ESP.getFreeHeap());
  Serial.printf("-------------------------------------------------\n\n");
}

#endif
