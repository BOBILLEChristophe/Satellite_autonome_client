/*


*/

#ifndef __LOCO_H__
#define __LOCO_H__

#include <Arduino.h>

class Loco
{
  
private:
  uint16_t m_address;
  uint8_t m_sens; // 0 pas de loco - 1 sens p - 2 sens m

public:
  Loco(); // Constructor
  void address(uint16_t);
  uint16_t address();
  void sens(uint8_t);
  uint8_t sens();
  
};

#endif
