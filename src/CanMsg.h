/*

  CanMsg.h


*/

#ifndef __CAN_MSG__
#define __CAN_MSG__

#include <ACAN_ESP32.h>
#include "Config.h"
#include "Discovery.h"
#include "Settings.h"

class CanMsg
{
public:
  CanMsg() = delete;
  static void setup(Node *);
  static void testMemory(void *);
  static void canReceiveMsg(void *);
  static void sendMsg(CANMessage &);
  static void sendMsg(byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
  static void sendMsg(byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte, byte);
};

#endif
