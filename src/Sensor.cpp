/*

  Sensor.cpp


*/

#include "Sensor.h"

void Sensor::setup(gpio_num_t pin, uint32_t tempo, byte input)
{
  m_pin = pin;
  m_tempo = tempo;
  m_input = input;
  pinMode(m_pin, m_input);
  xTaskCreate(this->loop, "loop", 1 * 1024, this, 3, NULL);
}

void IRAM_ATTR Sensor::loop(void *p)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  Sensor *pThis = (Sensor *)p;

  for (;;)
  {
    //if (!pThis->m_state) // Si l'état est à LOW
      pThis->m_state = ! digitalRead(pThis->m_pin);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200)); // toutes les x ms
  }
}

bool Sensor::state()
{
  return m_state;
}

void Sensor::state(bool state)
{
  m_state = state;
}
