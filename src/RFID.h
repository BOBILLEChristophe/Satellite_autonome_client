/*

  RFID.h


*/

#ifndef __RFID_H__
#define __RFID_H__

#include <Arduino.h>
#include "Config.h"
#include "MFRC522_I2C.h"
//#include "MFRC522_I2C.h" // https://github.com/arozcan/MFRC522-I2C-Library
                           // Voir aussi :https://github.com/semaf/MFRC522_I2C_Library/blob/master/examples/MFRC522_i2c.ino
#include <Wire.h>

// struct LocoTag
// {
//   uint16_t address;
//   uint16_t tag;
// };

class Rfid
{
private:

  uint16_t m_address;
  const gpio_num_t m_rstPin;
  const gpio_num_t m_sclPin;
  const gpio_num_t m_sdaPin;
  const uint64_t m_tempo;
  uint8_t m_nbLocos;
  static void IRAM_ATTR process(void *);

public:
  // Constructor
  Rfid(const gpio_num_t, const gpio_num_t, const gpio_num_t, const uint64_t);
  MFRC522_I2C  *mfrc522;
  void setup();
  //LocoTag locoTag[30];
  uint16_t address() const;
};

#endif
