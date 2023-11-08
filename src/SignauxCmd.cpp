/*



*/

#include "SignauxCmd.h"

// SignauxCmd::SignauxCmd() {}

gpio_num_t SignauxCmd::m_pinVerrou(GPIO_NUM_18);
gpio_num_t SignauxCmd::m_pinHorloge(GPIO_NUM_5);
gpio_num_t SignauxCmd::m_pinData(GPIO_NUM_4);
QueueHandle_t SignauxCmd::xQueueSignaux;

byte SignauxCmd::data[] = {
  0b00000000,
  0b00001000,
  0b00100000,
  0b01010000,
  0b00010100,
  0b00000011
};

void SignauxCmd::setup()
{
  pinMode(m_pinVerrou, OUTPUT);
  pinMode(m_pinHorloge, OUTPUT);
  pinMode(m_pinData, OUTPUT);
  //xQueueSignaux = queue;
}

void SignauxCmd::affiche(uint16_t x)
{
    digitalWrite(m_pinVerrou, LOW);
    shiftOut(m_pinData, m_pinHorloge, LSBFIRST, x);
    digitalWrite(m_pinVerrou, HIGH);
}

void IRAM_ATTR SignauxCmd::receiveData(void *p)
{
  uint8_t inByte;
  SignauxCmd *pThis = (SignauxCmd *)p;

  for (;;)
  {
    if (xQueueReceive(pThis->xQueueSignaux, &inByte, pdMS_TO_TICKS(portMAX_DELAY) == pdPASS))
    {
      pThis->affiche(inByte);
    }
    delay(1000);
  }
}