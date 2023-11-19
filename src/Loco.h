/*

  Loco.h


*/

#ifndef __LOCO_H__
#define __LOCO_H__

#include <Arduino.h>

class Loco
{

private:
  uint16_t m_address;
  bool m_sens; // 0 sens horaire - 1 sens anti horaire

public:
  Loco(); // Constructor
  void address(uint16_t);
  uint16_t address();
  void sens(bool);
  bool sens();

  uint8_t m_comptCmd0;
  uint8_t m_comptCmd30;
};

#endif
