/*

  Signal.cpp


*/

#include "Signal.h"

uint8_t Signal::m_compt(0);
uint16_t Signal::m_data(0);
uint16_t Signal::m_masque(0);

Signal::Signal() : m_position(0),
                   m_type(0),
                   m_length(0),
                   m_decalage(0)
{
}

Signal::Signal(bool position, uint8_t type) : m_position(position),
                                              m_type(type),
                                              m_length(0),
                                              m_decalage(0)
{
}

Signal::~Signal()
{
  m_compt = 0;
  m_data = 0;
  m_masque = 0;
}

void Signal::setup()
{
  m_decalage = m_compt;
  switch (m_type)
  {
  case 0: // Feu orange  - rouge - vert
    m_length = 3;
#ifdef DEBUG
    debug.printf("[Signal %d] : Creation d'un signal orange-vert-rouge\n", __LINE__);
#endif
    break;
  case 1: // Carré + oeilleton
    m_length = 5;
#ifdef DEBUG
    debug.printf("[Signal %d] : Creation d'un signal Carré + oeilleton\n", __LINE__);
#endif
    break;
  case 2: // Ralentissement (avec Carré + oeilleton)
    m_length = 7;
#ifdef DEBUG
    debug.printf("[Signal %d] : Creation d'un signal Ralentissement (avec Carré + oeilleton)\n", __LINE__);
#endif
    break;
  case 3: // RRalentissement (avec Carré + oeilleton)
    m_length = 7;
#ifdef DEBUG
    debug.printf("[Signal %d] : Creation d'un signal RRalentissement (avec Carré + oeilleton)\n", __LINE__);
#endif
    break;
  }
  m_compt += m_length;
}

uint16_t Signal::affiche(uint16_t x)
{

  uint16_t clearMasque = ((1 << m_length) - 1) << m_decalage;
  m_masque &= ~clearMasque;

  switch (m_type)
  {
  case 0:
    switch (x) // orange  - rouge - vert
    {
    case 0: // orange
    case 4: // ralentissement
      m_data = B1 << m_decalage;
      break;
    case 1: // rouge
      m_data = B10 << m_decalage;
      break;
    case 2: // vert
      m_data = B100 << m_decalage;
      break;
    }
    break;
  case 1: // Carré + oeilleton
    switch (x)
    {
    case 0: // orange - avertissement
    case 4: // ralentissement
      m_data = B00000010 << m_decalage;
      break;
    case 1: // rouge - sémaphore + oeilleton
      m_data = B00000101 << m_decalage;
      break;
    case 2: // vert - voie libre
      m_data = B00001000 << m_decalage;
      break;
    case 3: // rouge - carré
      m_data = B10100 << m_decalage;
      break;
    }
    break;
  case 2:
  case 3:
    switch (x)
    {
    case 0: // orange - avertissement
      m_data = B10 << m_decalage;
      break;
    case 1: // rouge - sémaphore + oeilleton
      m_data = B101 << m_decalage;
      break;
    case 2: // vert - voie libre
      m_data = B1000 << m_decalage;
      break;
    case 3: // rouge - carré
      m_data = B10100 << m_decalage;
      break;
    case 4: // ralentissement ou Rralentissement
      m_data = B1100000 << m_decalage;
      break;
    }
    break;
  }
  m_masque |= m_data;
  // debug.printf("[Signal %d] masque feux :", __LINE__);
  // Serial.println(m_masque, BIN);
  return m_masque;
}

void Signal::reset()
{
  m_compt = 0;
  m_data = 0;
}
void Signal::type(byte type) { m_type = type; }
byte Signal::type() { return m_type; }
void Signal::position(byte position) { m_position = position; }
byte Signal::position() { return m_position; }