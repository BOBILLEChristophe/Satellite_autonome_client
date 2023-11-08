/*

  Loco.cpp


*/

#include "Loco.h"

Loco::Loco() : m_address(0), m_sens(0)
{};

void Loco::address(uint16_t address) {m_address = address;}
uint16_t Loco::address() {return m_address;}
void Loco::sens(uint8_t sens) {m_sens = sens;}
uint8_t Loco::sens() {return m_sens;}

