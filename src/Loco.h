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
  uint8_t m_sens;               // 0 inderterminé - 1 sens horaire - 2 sens anti horaire
  uint8_t m_speed;

public:
  Loco(); // Constructor
  void address(uint16_t);
  uint16_t address();
  void sens(uint8_t);
  uint8_t sens();
  void speed(uint8_t);
  uint8_t speed();
  //void cmd();
  void ralentis(uint8_t);
  void stop();
};

#endif
