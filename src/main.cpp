/*

copyright (c) 2022 christophe.bobille - LOCODUINO - www.locoduino.org

*/

//---  Test si ESP32
#ifndef ARDUINO_ARCH_ESP32
#error "Select an ESP32 board"
#endif

#define PROJECT "Satellites autonomes (client)"
#define VERSION "v 0.9.12"
#define AUTHOR "christophe BOBILLE : christophe.bobille@gmail.com"

//--- Fichiers inclus

#include <Arduino.h>
#include "CanMsg.h"
#include "CanConfig.h"
#include "Config.h"
#ifdef CHIP_INFO
#include "ChipInfo.h"
#endif
#include "Discovery.h"
#include "GestionReseau.h"
#include "Node.h"
#ifdef RAILCOM
#include "Railcom.h"
#endif
#ifdef RFID
#include "RFID.h"
#endif
#include "Settings.h"
#include "SignauxCmd.h"
#include "WebHandler.h"
#include "Wifi_fl.h"
#include "freertos/queue.h"

// Instances
Node *node = new Node();
#ifdef RAILCOM
Railcom railcom(RAILCOM_RX, RAILCOM_TX);
#endif
#ifdef RFID
Rfid rfid(RST_PIN, SCL_PIN, SDA_PIN, TEMPO_RFID);
#endif
Fl_Wifi wifi;
WebHandler webHandler;

/*-------------------------------------------------------------
                           setup
--------------------------------------------------------------*/

void setup()
{

  Serial.begin(115200);
  while (!Serial)
    ;

  debug.printf("\nProject :    %s", PROJECT);
  debug.printf("\nVersion   :    %s", VERSION);
  debug.printf("\nAuteur    :    %s", AUTHOR);
  debug.printf("\nFichier   :    %s", __FILE__);
  debug.printf("\nCompiled  :    %s", __DATE__);
  debug.printf(" - %s\n\n", __TIME__);

//--- Infos ESP32 (desactivable)
#ifdef CHIP_INFO
  ChipInfo::print();
#endif

  Settings::setup(node);
  delay(100);
  //--- Configure ESP32 CAN
  CanConfig::setup(node->ID(), Settings::discoveryOn());
  delay(100);
  CanMsg::setup(node);
  delay(100);

  bool err = 0;
  if (err == Settings::begin())
  {
    debug.printf("-----------------------------------\n");
    debug.printf("ID Node : %d\n", node->ID());
    debug.printf("-----------------------------------\n");
  }
  else
  {
    debug.printf("[Settings] : Echec de la configuration\n");
    return;
  }

  //--- Wifi et web serveur
  if (Settings::wifiOn()) // Si option validée
  {
    wifi.start();
    webHandler.init(node, 80);
  }
  debug.printf(Settings::wifiOn() ? "[Wifi] : on\n" : "Wifi : off\n");

  //--- Lancement de la méthode pour le procecuss de découverte
  if (Settings::discoveryOn()) // Si option validée
    Discovery::begin(node);
  debug.printf(Settings::discoveryOn() ? "[Discovery] : on\n" : "[Discovery] : off\n");
  debug.println();
#ifdef RFID
  rfid.setup();
#endif

  //--- Signal::setup()
  // if (! Settings::discoveryOn())
  // {
  //   for (byte i = 0; i < aigSize; i++)
  //   {
  //     if (node->signal[i] != nullptr)
  //       node->signal[i]->setup(i);
  //   }
  // }

  GestionReseau::setup(node);
  debug.printf("[GestionResau] : setup\n\n");

} // ->End setup

/*-------------------------------------------------------------
                           loop
--------------------------------------------------------------*/

void loop()
{
  //******************** Ecouteur page web **********************************

  if (Settings::wifiOn()) // Si option validée
    webHandler.loop();    // ecoute des ports web 80 et 81

  if (!Settings::discoveryOn())
  {
    //************************* Railcom ****************************************

    if (railcom.address())
    {
      node->busy(true);
      node->loco.address(railcom.address());
      //debug.printf("[main %d ] Railcom - Numero de loco : %d\n", __LINE__, node->loco.address());
    }
    else
    {
      node->busy(false);
      node->loco.address(0);
      // debug.printf("Railcom - Pas de loco.\n");
    }

    //****************************** RFID **************************************
#ifdef RFID
    if (rfid.address())
      debug.printf("RFID - Numero de loco : %d\n", rfid.address());
    else
      debug.printf("RFID - Pas de loco.\n");
#endif
    //**************************************************************************
  }
  vTaskDelay(pdMS_TO_TICKS(10));
} // ->End loop
