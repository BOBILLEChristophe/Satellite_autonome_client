/*

  Discovery.h


*/

#ifndef __DISCOVERY_H__
#define __DISCOVERY_H__

#include <Arduino.h>
#include "CanMsg.h"
#include "Config.h"
#include "Node.h"
#include "Settings.h"

class Discovery
{
private:
  static const gpio_num_t m_pinIn[];
  static const gpio_num_t m_pinLed;
  static byte m_switchAig;
  static const gpio_num_t m_aigPin[];
  static Node *node;
  static byte m_comptAig;
  static byte m_ID_satPeriph;
  static byte m_btnState;
  static bool m_stopProcess;
  //static SemaphoreHandle_t xSemaphore;

public:
  Discovery() = delete;
  static void begin(Node *);
  static void process(void *);
  static void createAiguilles(void *);
  static void createCiblesSignaux(void *);
  static void comptAig(byte);
  static byte comptAig();
  static void ID_satPeriph(byte);
  static byte ID_satPeriph();
  static void btnState(byte);
  static byte btnState();
  static void stopProcess(bool);
};

#endif
