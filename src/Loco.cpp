/*

  Loco.cpp


*/

#include "Loco.h"

Loco::Loco() : m_address(0),
               m_sens(0),
               m_speed(0){};

void Loco::address(uint16_t address) { m_address = address; }
uint16_t Loco::address() { return m_address; }
void Loco::sens(uint8_t sens) { m_sens = sens; }
uint8_t Loco::sens() { return m_sens; }
void Loco::speed(uint8_t speed) { m_speed = speed; }
uint8_t Loco::speed() { return m_speed; }

void Loco::stop()
{
    m_speed = 0;
};

void Loco::ralentis(uint8_t speed)
{
    m_speed = speed;
};
