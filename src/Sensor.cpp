/*

  Sensor.cpp


*/

#include "Sensor.h"

Sensor::Sensor() : m_state(false){};
Sensor::~Sensor(){};

void Sensor::setup(gpio_num_t pin, uint32_t tempo, byte input)
{
  m_pin = pin;
  m_tempo = tempo;
  m_input = input;
  pinMode(m_pin, m_input);
  xTaskCreate(this->loop, "loop", 2 * 1024, this, 10, NULL);
}

void IRAM_ATTR Sensor::loop(void *p)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();
  Sensor *pThis = (Sensor *)p;

  for (;;)
  {
    if (!pThis->m_state) // Si l'état est à LOW
      pThis->m_state = !digitalRead(pThis->m_pin);
    // else
    //   debug.printf("[Sensor %d] Le capteur %d est actif\n", __LINE__, pThis->m_pin);
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(pThis->m_tempo)); // toutes les x ms
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
