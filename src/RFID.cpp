/*

  RFID.cpp


*/

#include "RFID.h"

/* ----- Constructeur   -------------------*/

Rfid::Rfid(const gpio_num_t rstPin,
           const gpio_num_t sclPin,
           const gpio_num_t sdaPin,
           const uint64_t tempo)
    : m_rstPin(rstPin),
      m_sclPin(sclPin),
      m_sdaPin(sdaPin),
      m_tempo(tempo),
      m_address(0),
      mfrc522(nullptr)
{}

void Rfid::setup()
{
  mfrc522 = new MFRC522_I2C(0x28, m_rstPin); // MFRC522 instance.
  Wire.begin(m_sdaPin, m_sclPin);
  mfrc522->PCD_Init(); // Initialize MFRC522
  //  Show details of PCD - MFRC522 Card Reader details
  byte v = mfrc522->PCD_ReadRegister(mfrc522->VersionReg);
  debug.printf("MFRC522 Software Version: %0Xv", v);
  if (v == 0x91)
    debug.printf(" = v1.0\n");
  else if (v == 0x92)
    debug.printf(" = v2.0\n");
  else
    debug.printf(" (unknown)\n");
  //  When 0x00 or 0xFF is returned, communication probably failed
  if ((v == 0x00) || (v == 0xFF))
    debug.printf("WARNING: Communication failure, is the MFRC522 properly connected ?\n");

  TaskHandle_t process = NULL;
  //xTaskCreatePinnedToCore(this->process, "Process", 2 * 1024, this, 3, &process, 1); // Création de la tâches pour MAJ adresse
}

void IRAM_ATTR Rfid::process(void *p)
{
  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  Rfid *pThis = (Rfid *)p;

  const uint16_t tabLoco[][2]{
      {10, 0xC337},
      {11, 0x7338},
      {16, 0x4637},
      {30, 0xC437},
      {65, 0x0437}};

  auto checkTag = [&](uint16_t tag) -> uint16_t
  {
    for (auto el : tabLoco)
    {
      if (tag == el[1])
        return el[0];
    }
    return 0;
  };

//!\\ Il faudra mettre m_address à 0 quand le canton est libre (busy == false)
 
  for (;;)
  {
    if (pThis->mfrc522->PICC_ReadCardSerial() || pThis->mfrc522->PICC_IsNewCardPresent())
    {
      if (0x04 == pThis->mfrc522->uid.uidByte[0])
        pThis->m_address = checkTag((pThis->mfrc522->uid.uidByte[1]) << 8) | (pThis->mfrc522->uid.uidByte[2]);
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1)); // toutes les x ms
  }
}

/* ----- getAddress   -------------------*/

uint16_t Rfid::address() const
{
  return m_address;
}