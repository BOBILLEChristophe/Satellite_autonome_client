/*

Aig.cpp


*/

#include "Aig.h"

uint8_t Aig::m_compt(0);

Aig::Aig() : m_posDroit(1500),
             m_posDevie(1500),
             m_minPos(800),
             m_maxPos(2400),
             m_servoPin(NO_PIN),
             m_aPointe(true),
             m_estDroit(true),
             m_run(false),
             m_sens(HIGH),
             m_curPos(0),
             m_speed(AIG_SPEED){};

Aig::~Aig(){};

void Aig::setup()
{
  attach(m_servoPin);
  m_curPos = m_posDroit;
  move(m_posDroit);
  m_estDroit = true;
  delay(1000);
}

void Aig::move(const uint16_t pos)
{
  if ((pos > m_minPos) && (pos < m_maxPos))
    writeMicroseconds(pos);
}

void Aig::speed(uint16_t speed)
{
  if (speed > 0 && speed <= 10000)
    m_speed = speed;
  else
    m_speed = 5000;
}

void Aig::posDroit(const uint16_t pos)
{
  if (pos > m_minPos && pos < m_maxPos)
    m_posDroit = pos;
}

void Aig::posDevie(const uint16_t pos)
{
  if (pos > m_minPos && pos < m_maxPos)
    m_posDevie = pos;
}

void Aig::curPos(const uint16_t curPos)
{
  if (curPos > m_minPos && curPos < m_maxPos)
    m_curPos = curPos;
}

void Aig::ID(const uint8_t id) { m_id = id; }
uint8_t Aig::ID() const { return m_id; }
void Aig::estDroit(const bool pos) { m_estDroit = pos; }
bool Aig::estDroit() const { return m_estDroit; }
void Aig::signalPin(const byte pin) { m_servoPin = pin; }
uint8_t Aig::signalPin() const { return m_servoPin; }
uint16_t Aig::speed() const { return m_speed; }
uint16_t Aig::posDroit() const { return m_posDroit; }
uint16_t Aig::posDevie() const { return m_posDevie; }
uint16_t Aig::curPos() const { return m_curPos; }
bool Aig::sens() const { return m_sens; }
void Aig::sens(const bool sens) { m_sens = sens; }
bool Aig::isRunning() const { return m_run; }
void Aig::run(const bool run) { m_run = run; }
