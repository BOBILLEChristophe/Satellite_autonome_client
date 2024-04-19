/*

copyright (c) 2022 christophe.bobille - LOCODUINO - www.locoduino.org

v 0.11.8 : Ajout de la détection de présence par consommation de courant
v 0.11.9 : Correction de divers petits bugs après essais sur réseau

*/

//---  Test si ESP32
#ifndef ARDUINO_ARCH_ESP32
#error "Select an ESP32 board"
#endif

#define PROJECT "Satellites autonomes (client)"
#define VERSION "v 0.12.4"
#define AUTHOR "christophe BOBILLE : christophe.bobille@gmail.com"

//--- Fichiers inclus
#include <Arduino.h>
#include "CanMsg.h"
#include "CanConfig.h"
#include "Config.h"
#ifdef CHIP_INFO
#include "ChipInfo.h"
#endif
#include "ConsoCourant.h"
#include "Discovery.h"
#include "GestionReseau.h"
#include "Node.h"
#include "Railcom.h"
#include "Settings.h"
#include "SignauxCmd.h"
#include "WebHandler.h"
#include "Wifi_fl.h"
#include "freertos/queue.h"

// Instances
Node *node = new Node();
Railcom railcom(RAILCOM_RX, RAILCOM_TX);
Fl_Wifi wifi;
WebHandler webHandler;
ConsoCourant consoCourant;

/*-------------------------------------------------------------
                           setup
--------------------------------------------------------------*/

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
  delay(100);

//--- Infos ESP32 (desactivable)
#ifdef CHIP_INFO
  ChipInfo::print();
#endif

  Serial.printf("\nProject   :    %s", PROJECT);
  Serial.printf("\nVersion   :    %s", VERSION);
  Serial.printf("\nAuteur    :    %s", AUTHOR);
  Serial.printf("\nFichier   :    %s", __FILE__);
  Serial.printf("\nCompiled  :    %s", __DATE__);
  Serial.printf(" - %s\n\n", __TIME__);
  Serial.printf("-----------------------------------\n\n");

  Settings::setup(node);
  vTaskDelay(pdMS_TO_TICKS(100));
  //--- Configure ESP32 CAN
  CanConfig::setup();
  vTaskDelay(pdMS_TO_TICKS(100));
  CanMsg::setup(node);
  vTaskDelay(pdMS_TO_TICKS(100));

  bool err = 0;
  if (err == Settings::begin())
  {
    Serial.printf("-----------------------------------\n");
    Serial.printf("ID Node : %d\n", node->ID());
    Serial.printf("-----------------------------------\n\n");
  }
  else
  {
    Serial.printf("[Settings] : Echec de la configuration\n");
    return;
  }

  if (Settings::discoveryOn()) // Si option validee, lancement de la méthode pour le procecuss de decouverte
  {
    Discovery::begin(node);
    Settings::wifiOn(true);
  }
  else
  {
    // Settings::wifiOn(false);
    for (byte i = 0; i < signalSize; i++)
    {
      if (node->signal[i] == nullptr)
        node->signal[i] = new Signal;
      node->signal[i]->setup();
    }
    railcom.begin();
    SignauxCmd::setup();
    GestionReseau::setup(node);
    consoCourant.setup(node, CONSO_COURANT_PIN);
  }
  //--- Wifi et web serveur
  if (Settings::wifiOn()) // Si option validee
  {
    wifi.start();
    webHandler.init(node, 80);
  }

  Serial.printf(Settings::discoveryOn() ? "[Discovery] : on\n" : "[Discovery] : off\n");
  Serial.printf(Settings::wifiOn() ? "[Wifi] : on\n" : "Wifi : off\n");
  Serial.printf("-----------------------------------\n");
  Serial.printf("[Main %d] : End setup\n\n", __LINE__);
  Serial.printf("-----------------------------------\n\n");
#ifndef debug
  vTaskDelay(pdMS_TO_TICKS(1000));
  Serial.end(); // Desactivation de Serial
#endif
} // ->End setup

/*-------------------------------------------------------------
                           loop
--------------------------------------------------------------*/

void loop()
{
  //******************** Ecouteur page web **********************************

  if (Settings::wifiOn()) // Si option validée
    webHandler.loop();    // ecoute des ports web 80 et 81

  if (!Settings::discoveryOn()) // Si option non validée
  {
    //************************* Railcom ****************************************

    if (railcom.address() && node->busy())
    {
      node->loco.address(railcom.address());
      // #ifdef debug
      //       debug.printf("[Main %d] Railcom - Numero de loco : %d\n", __LINE__, node->loco.address());
      // #endif
    }
  }
  //**************************************************************************
  vTaskDelay(pdMS_TO_TICKS(100));
} // ->End loop
