/*

  ConsoCourant.h


*/

#ifndef __CONSO_COURANT__
#define __CONSO_COURANT__

#include <Arduino.h>
#include "Config.h"

class ConsoCourant
{
private:
  gpio_num_t m_pinIn;

public:
  ConsoCourant();
  ~ConsoCourant();
  Node *m_node;
  void setup(Node *, const gpio_num_t);
  static void IRAM_ATTR loop(void *pvParameters);
};

ConsoCourant::ConsoCourant(){};
ConsoCourant::~ConsoCourant(){};

void ConsoCourant::setup(Node *node, const gpio_num_t pinIn)
{
  m_node = node;
  m_pinIn = pinIn;
  pinMode(m_pinIn, INPUT_PULLUP);
  xTaskCreate(this->loop, "loop", 1 * 1024, this, 10, NULL);
}

void IRAM_ATTR ConsoCourant::loop(void *p)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  ConsoCourant *pThis = (ConsoCourant *)p;

  for (;;)
  {
    if (digitalRead(pThis->m_pinIn)) 
      pThis->m_node->busy(false);
    else
      pThis->m_node->busy(true);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100)); // toutes les x ms
  }
}
#endif
