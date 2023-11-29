/*

  SignauxCmd.h


*/

#ifndef __SIGN_CMD_H__
#define __SIGN_CMD_H__

#include <Arduino.h>
#include "Config.h"

class SignauxCmd
{
  private:
    static gpio_num_t m_pinVerrou;
    static gpio_num_t m_pinHorloge;
    static gpio_num_t m_pinData;

  public:
    SignauxCmd() = delete;
    static void setup();
    static void affiche(uint16_t);
};

#endif