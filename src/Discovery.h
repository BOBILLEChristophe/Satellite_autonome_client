/*


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
  static bool m_toggleLed;
  static byte m_switchAig;
  static const byte m_aigPin[];
  static Node *node;
  static bool m_retS0_S1;
  static byte m_comptAig;
  static byte m_thisID_sat;
  static byte m_ID_satPeriph;
  static byte m_btnState;

public:
  Discovery() = delete;
  static void begin(Node *);
  static void process(void *);
  static void createAiguilles();
  static void createCiblesSignaux(void *);
  static void comptAig(byte);
  static byte comptAig();
  static void retS0_S1(bool);
  static bool retS0_S1();
  static void ID_satPeriph(byte);
  static byte ID_satPeriph();
  static void thisID_sat(byte);
  static byte thisID_sat();
  static void btnState(byte);
  static byte btnState();
};

#endif
