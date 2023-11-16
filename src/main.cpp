/*

copyright (c) 2022 christophe.bobille - LOCODUINO - www.locoduino.org

*/

//---  Test si ESP32
#ifndef ARDUINO_ARCH_ESP32
#error "Select an ESP32 board"
#endif

#define PROJECT "Satellites autonomes (client)"
#define VERSION "v 0.9.10"
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

  //--- Start serial
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

  //--- Configure ESP32 CAN
  CanConfig::setup(node->ID(), Settings::discoveryOn());
  CanMsg::setup(node);

  bool err = 0;
  if (err == Settings::begin())
  {
    debug.printf("-----------------------------------\n");
    debug.printf("ID Node : %d\n", node->ID());
    debug.printf("-----------------------------------\n");
  }
  else
  {
    debug.printf("Echec de la configuration\n");
    return;
  }

  //--- Wifi et web serveur
  if (Settings::wifiOn()) // Si option validée
  {
    wifi.start();
    webHandler.init(node, 80);
  }
  debug.printf(Settings::wifiOn() ? "Wifi : on\n" : "Wifi : off\n");

  //--- Lancement de la méthode pour le procecuss de découverte
  if (Settings::discoveryOn()) // Si option validée
    Discovery::begin(node);
  debug.printf(Settings::discoveryOn() ? "Discovery : on\n" : "Discovery : off\n");
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

  // Test
  // if (node.nodeP[1] == nullptr)
  // {
  //   node.nodeP[1] = new NodePeriph;
  //   node.nodeP[1]->ID(12);
  // }
  // Serial.printf("Instance(s) de NodePeriph = %d\n", NodePeriph::comptInst);
  // delay(1000);
  // // Serial.printf("node.nodeP[1]->ID() = %d\n", node.nodeP[1]->ID());
  // for (byte i = 0; i < aigSize; i++)
  // {
  //   if (node.nodeP[i] != nullptr)
  //   {
  //     delete node.nodeP[i];
  //     node.nodeP[i] = nullptr;
  //   }
  // }
  // Serial.printf("Instance(s) de NodePeriph = %d\n", NodePeriph::comptInst);

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
#ifdef RAILCOM
    if (railcom.address())
    {
      node->busy(true);
      node->loco.address(railcom.address());
      // debug.printf("Railcom - Numero de loco : %d\n", railcom.address());
    }
    else
    {
      node->busy(false);
      node->loco.address(0);
      // debug.printf("Railcom - Pas de loco.\n");
    }
#endif
    //**************************************************************************

    //****************************** RFID **************************************
#ifdef RFID
    if (rfid.address())
      debug.printf("RFID - Numero de loco : %d\n", rfid.address());
    else
      debug.printf("RFID - Pas de loco.\n");
#endif
    //**************************************************************************

    //****************************** Capteurs IR **************************************
    // if (node->sensor[0].state())
    //   debug.println("Capteur horaire actif.");

    // if (node->sensor[1].state())
    //   debug.println("Capteur anti-horaire actif.");
  }

  vTaskDelay(pdMS_TO_TICKS(10));
} // ->End loop
