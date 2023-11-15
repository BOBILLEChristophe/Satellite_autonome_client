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
  uint8_t m_comptCmd0;
  uint8_t m_comptCmd30;

public:
  Loco(); // Constructor
  void address(uint16_t);
  uint16_t address();
  void sens(bool);
  bool sens();
  void comptCmd0(uint8_t);
  uint8_t comptCmd0();
  void comptCmd30(uint8_t);
  uint8_t comptCmd30();
};

#endif
