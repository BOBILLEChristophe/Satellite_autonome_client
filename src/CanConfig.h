/*


  CanConfig.h
  

*/

#ifndef __CAN_CONFIG__
#define __CAN_CONFIG__

#include <Arduino.h>
#include <ACAN_ESP32.h>
#include "Config.h"

class CanConfig
{
private:
public:
  CanConfig() = delete;
  static void setup(const byte);
  //static void setup(const byte, const bool);
};

#endif