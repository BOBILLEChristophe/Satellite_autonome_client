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
    

  public:
    SignauxCmd();
    static byte data[6];
    static gpio_num_t m_pinVerrou;
    static gpio_num_t m_pinHorloge;
    static gpio_num_t m_pinData;
    static QueueHandle_t xQueueSignaux;
    static void affiche(uint16_t x);
    static void setup();
    //static void setup(QueueHandle_t queue);
    static void IRAM_ATTR receiveData(void *p);
};

#endif