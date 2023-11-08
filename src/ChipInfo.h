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

void ChipInfo::print() {
  //--- Display ESP32 Chip Info
  esp_chip_info_t chip_info ;
  esp_chip_info (&chip_info) ;
  debug.printf ("\n\n---------------Infos ESP32----------------------\n");
  debug.printf ("Arduino Release : %s\n", ARDUINO_ESP32_RELEASE);
  debug.printf ("Chip Revision : %d\n", chip_info.revision);
  debug.printf ("SDK : %s\n", ESP.getSdkVersion ());
  debug.printf("CPU freq : %d MHz\n", ESP.getCpuFreqMHz());
  debug.printf("CPU cores : %d\n", chip_info.cores);
  debug.printf ("Flash size :  %d MB ", spi_flash_get_chip_size () / (1024 * 1024)) ;
  debug.println(((chip_info.features & CHIP_FEATURE_EMB_FLASH) != 0) ? "(embeded)" : "(external)");
  debug.printf ("APB CLOCK : %d MHz\n", APB_CLK_FREQ / (1000 * 1000)) ;
  debug.printf("Free RAM : %ld bytes\n", (long)ESP.getFreeHeap());
  debug.printf ("-------------------------------------------------\n\n");
}

#endif
