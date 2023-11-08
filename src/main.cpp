/*

copyright (c) 2022 christophe.bobille - LOCODUINO - www.locoduino.org

*/

//---  Test si ESP32
#ifndef ARDUINO_ARCH_ESP32
#error "Select an ESP32 board"
#endif

#define PROJECT "Satellites autonomes (client)"
#define VERSION "v 0.9.6"
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
Node node;
#ifdef RAILCOM
Railcom railcom(RAILCOM_RX, RAILCOM_TX);
#endif
#ifdef RFID
Rfid rfid(RST_PIN, SCL_PIN, SDA_PIN, TEMPO_RFID);
#endif
Fl_Wifi wifi;
WebHandler webHandler;

void gestionReseau();

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

  Settings::setup(&node);

  //--- Configure ESP32 CAN
  CanConfig::setup(node.ID(), Settings::discoveryOn());
  CanMsg::setup(&node);

  bool err = 0;
  if (err == Settings::begin())
  {
    debug.printf("-----------------------------------\n");
    debug.printf("ID Node : %d\n", node.ID());
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
    webHandler.init(&node, 80);
  }
  debug.printf(Settings::wifiOn() ? "Wifi : on\n" : "Wifi : off\n");

  //--- Lancement de la méthode pour le procecuss de découverte
  if (Settings::discoveryOn()) // Si option validée
    Discovery::begin(&node);
  debug.printf(Settings::discoveryOn() ? "Discovery : on\n" : "Discovery : off\n");
  debug.println();
#ifdef RFID
  rfid.setup();
#endif

  // Signal::setup()
  for (auto el : node.signal)
  {
    if (el != nullptr)
      el->setup();
  }

  GestionReseau::setup(&node);

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
    //******************** Gestion du réseau **********************************
    // gestionReseau();
    // GestionReseau::run(true);
    //************************* Railcom ****************************************
#ifdef RAILCOM
    if (railcom.address())
    {
      node.busy(true);
      node.loco.address(railcom.address());
      // debug.printf("Railcom - Numero de loco : %d\n", railcom.address());
    }
    else
    {
      node.busy(false);
      node.loco.address(0);
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
    if (node.sensor[0].state())
    {
      debug.println("Capteur horaire actif.");
    }

    if (node.sensor[1].state())
    {
      debug.println("Capteur anti-horaire actif.");
    }
  }

  vTaskDelay(pdMS_TO_TICKS(10));
} // ->End loop

void gestionReseau()
{
  auto rechercheSat = [](bool satPos) -> byte
  {
    uint8_t idxA = 0;
    uint8_t idxS = 0;

    if (satPos == 1)
    {
      idxA += 3;
      idxS += 4;
    }

    // byte SAT = NO_ID;
    byte SAT = 0 + idxS;

    if (node.aig[0 + idxA] != nullptr) // L'aiguille 0 à horaire (ou 3 à anti-horaire) existe
    {
      if (node.aig[0 + idxA]->estDroit()) // L'aiguille 0 à horaire (ou 3 à anti-horaire) est droite
      {
        SAT = 0 + idxS;
        if (node.aig[1 + idxA] != nullptr) // L'aiguille 1 à horaire (ou 4 à anti-horaire) existe
        {
          if (!node.aig[1 + idxA]->estDroit()) // L'aiguille 1 à horaire (ou 4 à anti-horaire) est droite
            SAT = 0 + idxS;
          else
            SAT = 1 + idxS; // L'aiguille 1 à horaire (ou 4 à anti-horaire) est déviée
        }
      }
      else // L'aiguille 0 à horaire (ou 3 à anti-horaire) est déviée
      {
        SAT = 1 + idxS;
        if (node.aig[2 + idxA] != nullptr) // L'aiguille 2 à horaire (ou 5 à anti-horaire) existe
        {
          if (node.aig[2 + idxA]->estDroit())
            SAT = 2 + idxS; // L'aiguille 2 à horaire (ou 5 à anti-horaire) est droite
          else
            SAT = 3 + idxS; // L'aiguille 2 à horaire (ou 5 à anti-horaire) est déviée
        }
      }
    }
    return SAT;
  };
  //***************************************************************************

  /*********** Recherche du SP1 et du SM1 pour ce satellite
           en fonction des aiguilles existantes et de leur position **********/

  node.SP1_idx(rechercheSat(0));
  node.SM1_idx(rechercheSat(1));

#ifdef DEBUG
  // debug.printf("node.SP1_idx() %d\n", node.SP1_idx());
  // debug.printf("node.SM1_idx() %d\n", node.SM1_idx());
#endif

  // Pour déterminer le sens de roulage des locos, on demande à SP1 et à SM1
  // Quelle est la loco qui l'occupe (s'il y en a une) et on enregistre cette info
  // Ensuite, on compare ces adresses avec l'adresse locale

  // Ce satellite envoie une demande de confirmation à son SP1 et son SM1
  const byte spIdx[2] = {node.SP1_idx(), node.SM1_idx()};
  for (byte i = 0; i < 2; i++)
  {
    if (node.nodeP[spIdx[i]] != nullptr)
      CanMsg::sendMsg(0, node.ID(), node.nodeP[spIdx[i]]->ID(), 0xA1 + i);
  }

  if (node.nodeP[node.SP1_idx()] != nullptr)
  {
    CanMsg::sendMsg(0, node.ID(), node.nodeP[node.SP1_idx()]->ID(), 0xA1);
    // debug.printf("node.nodeP[node.SP1_idx()]->busy() %d\n", node.nodeP[node.SP1_idx()]->busy());
    // debug.printf("node.nodeP[node.SP1_idx()]->acces() %d\n", node.nodeP[node.SP1_idx()]->acces());
  }
  if (node.nodeP[node.SM1_idx()] != nullptr)
  {
    CanMsg::sendMsg(0, node.ID(), node.nodeP[node.SM1_idx()]->ID(), 0xA2);
    // debug.printf("node.nodeP[node.SM1_idx()]->busy() %d\n", node.nodeP[node.SM1_idx()]->busy());
    // debug.printf("node.nodeP[node.SM1_idx()]->acces() %d\n", node.nodeP[node.SM1_idx()]->acces());
  }

  //**************************************************************************

  //****************** Commande des locomotives ******************************

  auto cmdLoco = [](const uint16_t address, const uint8_t speed, const uint8_t direction)
  {
    const uint8_t $address0 = address & 0x00FF;
    const uint8_t $address1 = address & 0xFF00;
    CanMsg::sendMsg(1, node.ID(), 253, 0xF0, $address1, $address0, speed, direction); // Message à la centrale DCC++
  };

  //   //************************* Process ****************************************

  enum
  {
    rouge,
    vert,
    orange,
    carre
  };

  static byte comptLoco = 0;
  if (comptLoco == 5)
    comptLoco = 0;

  if (node.SP1_idx() < 253)
  {
    if (node.nodeP[node.SP1_idx()] != nullptr)
    {
      // debug.printf("canton SP1 : %d\n", node.nodeP[node.SP1_idx()]->ID());
      if (node.nodeP[node.SP1_idx()]->acces()) // Le canton SP1 est accessible
      {
        debug.printf("Le canton SP1 est accessible\n");
        if (node.nodeP[node.SP1_idx()]->busy()) // Le canton SP1 est occupé
        {
          debug.printf("Le canton SP1 est occupe\n");
          //  node.signal[0]->affiche(rouge); // Signalisation Rouge + oeilleton
          //  Ordre loco Ralentissement à 30
          if (node.sensor[1].state())
          {
            if (comptLoco < 5)
            {
              cmdLoco(node.loco.address(), 30, 1);
              comptLoco++;
            }
          }
          // arret au franchissement du capteur
          if (node.sensor[0].state())
          {
            if (comptLoco < 5)
            {
              cmdLoco(node.loco.address(), 0, 1);
              comptLoco++;
            }
          }
        }
        else // Le canton SP1 est accessible et libre
        {    // Vérification de l'état de SP2
          debug.printf("SP1 = node.nodeP[%d] : libre\n", node.SP1_idx());

          debug.printf("Le canton SP1 est accessible et libre\n");
          CanMsg::sendMsg(0, node.ID(), 0, 0xA4, node.nodeP[node.SP1_idx()]->ID());
          if (node.SP2_acces()) // Le canton SP2 est-il accessible ?
          {
            // debug.printf("SP2 accessible\n");
            if (node.SP2_busy()) // Le canton SP2 est-il occupé ?
            {
              //  debug.printf("SP2 occupe\n");
              //  Signalisation Ralentissement
              //  Ordre loco Ralentissement à 30
              if (node.sensor[1].state())
              {
                if (comptLoco < 5)
                {
                  cmdLoco(node.loco.address(), 30, 1);
                  comptLoco++;
                }
              }
            }
            else // Le canton SP2 n'est pas occupé
            {
              // Signalisation vert
            }
          }
          else // Le canton SP2 n'est pas accessible
          {
            // Signalisation Ralentissement
            // Ordre loco Ralentissement à 30
            if (node.sensor[1].state())
            {
              if (comptLoco < 5)
              {
                cmdLoco(node.loco.address(), 30, 1);
                comptLoco++;
              }
            }
          }
        }
      }
    }
    else // Le canton SP1 n'est pas accessible
    {
      debug.printf("Le canton SP1 n'est pas accessible\n");
      // Signalisation Carré
      // Ordre loco Ralentissement
      if (node.sensor[1].state())
      {
        if (comptLoco < 5)
        {
          cmdLoco(node.loco.address(), 30, 1);
          comptLoco++;
        }
      }
      // Ordre loco Arret
      if (node.sensor[0].state())
      {
        if (comptLoco < 5)
        {
          cmdLoco(node.loco.address(), 0, 1);
          comptLoco++;
        }
      }
    }
  }
  else
  {
    /* Gestion du cas où il n'y a pas de node SP1

     Ce peut être le cas où le satellite suivant n'existe pas encore
     ou c'est une voie de garage */

    // Signalisation Carré
    if (node.sensor[0].state())
    {
      if (comptLoco < 5)
      {
        cmdLoco(node.loco.address(), 0, 1);
        comptLoco++;
      }
    }
  }
}
