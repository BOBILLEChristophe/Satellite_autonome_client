/*

  Loco.cpp


*/

#include "Loco.h"

Loco::Loco() : m_address(0), m_sens(0), m_comptCmd0(0), m_comptCmd30(0)
{};

void Loco::address(uint16_t address) {m_address = address;}
uint16_t Loco::address() {return m_address;}
void Loco::sens(bool sens) {m_sens = sens;}
bool Loco::sens() {return m_sens;}
// void Loco::comptCmd0(uint8_t x) {m_comptCmd0 = x;}
// uint8_t Loco::comptCmd0() {return m_comptCmd0;}
// void Loco::comptCmd30(uint8_t x) {m_comptCmd30 = x;}
// uint8_t Loco::comptCmd30() {return m_comptCmd30;}

