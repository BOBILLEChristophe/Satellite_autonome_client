/*

  SignauxCmd.cpp


*/

#include "SignauxCmd.h"


gpio_num_t SignauxCmd::m_pinVerrou(GPIO_NUM_18);
gpio_num_t SignauxCmd::m_pinHorloge(GPIO_NUM_5);
gpio_num_t SignauxCmd::m_pinData(GPIO_NUM_4);

void SignauxCmd::setup()
{
  //debug.printf("[SignauxCmd %d] : setup\n", __LINE__);
  pinMode(m_pinVerrou, OUTPUT);
  pinMode(m_pinHorloge, OUTPUT);
  pinMode(m_pinData, OUTPUT);
}

void SignauxCmd::affiche(uint16_t data)
{
    digitalWrite(m_pinVerrou, LOW);
    shiftOut(m_pinData, m_pinHorloge, LSBFIRST, data);
    digitalWrite(m_pinVerrou, HIGH);
    // debug.printf("[SignauxCmd %d] : affiche : ", __LINE__);
    // debug.println(data, BIN);
}