/*

  Signal.cpp

https://www.jonroma.net/media/rail/opdocs/world/france/S%201%20A%20I-%20Signalisation%20au%20sol.pdf

*/

#ifndef __SIGN_H__
#define __SIGN_H__

#include <Arduino.h>
#include "Config.h"

class Signal
{
private:
  uint8_t m_id;
  uint8_t m_type;
  uint8_t m_length;
  uint16_t m_decalage;
  static uint8_t m_compt;
  static uint16_t m_data;
  static uint16_t m_masque;

public:
  Signal();
  void ID(const uint8_t);
  void setup(const uint8_t);
  static void reset();
  uint16_t affiche(const uint16_t);
  void type(byte);
  byte type();
};

#endif