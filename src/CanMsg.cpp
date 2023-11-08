/*



*/

#include "CanMsg.h"

void CanMsg::setup(Node *node)
{
  debug.println("CanMsg::setup");
  TaskHandle_t canReceiveHandle = NULL;
  xTaskCreate(canReceiveMsg, "CanReceiveMsg", 4 * 1024, (void *)node, 6, &canReceiveHandle); // Création de la tâches pour le traitement
#ifdef TEST_MEMORY_TASK
   xTaskCreate(testMemory, "TestMemory", 4 * 1024, (void *)canReceiveHandle, 2, NULL); // Création de la tâches pour le traitement
#endif
}

#ifdef TEST_MEMORY_TASK
void CanMsg::testMemory(void *pvParameters)
{
  UBaseType_t canReceiveMsg = 0;
  for (;;)
  {
    TaskHandle_t canReceiveHandle;
    canReceiveHandle = pvParameters;
    canReceiveMsg = uxTaskGetStackHighWaterMark(canReceiveHandle);
    debug.printf("canReceiveMsg free memory = % d bytes\n", canReceiveMsg);
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}
#endif

/*--------------------------------------
  Reception
  --------------------------------------*/
void CanMsg::canReceiveMsg(void *pvParameters)
{
  Node *node;
  node = (Node *)pvParameters;

  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    CANMessage frameIn;
    if (ACAN_ESP32::can.receive(frameIn))
    {
      const byte idSatExpediteur = (frameIn.id & 0x7F80000) >> 19; // ID du satellite qui envoie
      const byte *idSatDestinataire = &idSatExpediteur;
      const byte fonction = (frameIn.id & 0x7F8) >> 3;
#ifdef DEBUG
      //debug.printf("\n------ Expediteur %d : Fonction 0x%0X, Destinataire %d \n", idSatExpediteur, fonction, (frameIn.id & 0X7F800) >> 11);
#endif
      if (frameIn.rtr) // Remote frame
      {
        debug.printf("Frame de remote\n");
        switch (fonction)
        {
        case 0x0F:
          ACAN_ESP32::can.tryToSend(frameIn);
          break;
        }
      }
      else
      {
        switch (idSatExpediteur)
        {
        case 254:           // Envoi en provenance de la carte Main
          switch (fonction) // Fonction appelée
          {
          case 0xB3: // fn : Reponse à demande de test du bus CAN
            if (frameIn.data[0])
            {
              Settings::sMainReady(true);
#ifdef DEBUG
              debug.printf("------ Rec->Test bus CAN : OK\n");
#endif
            }
            break;

          case 0xB5: // fn : Reponse à demande d'identifiant (0xB4) faite par ce sat
            if (node->ID() == NO_ID)
            {
              node->ID(frameIn.data[0]);
#ifdef DEBUG
              debug.printf("------ Rec->ID node : %d\n", frameIn.data[0]);
#endif
            }
            break;

          case 0xB7: // fn : Reponse à demande nombre de locos (0xB6) faite par ce sat
            Settings::sNbLoco(frameIn.data[0]);
#ifdef DEBUG
            debug.printf("------ Rec->Nombre de locos : %d\n", frameIn.data[0]);
#endif
            break;

          case 0xB9: // fn : Reponse à tags des locos
            // node->rfid->tableLoco(
            //     frameIn.data[1],
            //     frameIn.data[2],
            //     frameIn.data[3],
            //     frameIn.data[4],
            //     frameIn.data[5],
            //     frameIn.data[6],
            //     frameIn.data[7]);
            // #ifdef DEBUG
            //           debug.printf("------ Rec->Loco n° %d\n", frameIn.data[1]);
            // #endif
            break;

          case 0xBD: // Activation du WiFi
                     // if (node->ID() == frameIn.data[0])
            Settings::wifiOn(frameIn.data[0]);
            break;

          case 0xBE: // Activation du mode Discovery
                     // if (node->ID() == frameIn.data[0])
            Settings::discoveryOn(frameIn.data[0]);
            break;

          case 0xBF: // fn : Enreistrement périodique des données en mémoire flash
#ifdef SAUV_BY_MAIN
#ifdef DEBUG
            debug.printf("------ Rec->sauvegarde automatique\n");
#endif
            Settings::writeFile();
            break;
#else
#ifdef DEBUG
            debug.printf("Sauvegarde automatique desactivee.\n");
#endif
#endif
          }
        default:            // Messages en provenance des autres satellites que le Main
          switch (fonction) // fonction appelée
          {
          case 0xA1: // Demande d'informations destiné à SP1
                     // if (node->ID() == frameIn.data[1]) // Si l'ID de ce sat correspond au SP1 de l'expéditeur
            CanMsg::sendMsg(0, node->ID(), *idSatDestinataire, 0xA3, node->SM1_idx(), node->busy(),
                           node->loco.address() & 0x00FF, node->loco.address() & 0xFF00); // SP1 envoie l'ID de son SM1, son état d'occupation et l'adresse de la loco
            break;

          case 0xA2: // Demande d'informations destiné à SM1
                     // if (node->ID() == frameIn.data[1]) // Si l'ID de ce sat correspond au SM1 de l'expéditeur
            CanMsg::sendMsg(0, node->ID(), *idSatDestinataire, 0xA4, node->SP1_idx(), node->busy(),
                            node->loco.address() & 0x00FF, node->loco.address() & 0xFF00); // SM1 envoie l'ID de son SPI, son état d'occupation et l'adresse de la loco
            break;

          case 0xA3:
            if (node->nodeP[node->SP1_idx()] != nullptr)
            {
              node->nodeP[node->SP1_idx()]->busy((bool)frameIn.data[1]); // Information de l'état occupé ou non de SP1
              uint16_t locoAddr = frameIn.data[2] << 8 | frameIn.data[3];
              if (node->SM1_idx() == frameIn.data[0])      // Si le SP1 de ce sat est celui envoyé
                node->nodeP[node->SP1_idx()]->acces(true); // Le SP1 est accessible
              else
                node->nodeP[node->SP1_idx()]->acces(false); // Le SP1 n'est pas accessible
            }
            break;

          case 0xA4:
            if (node->nodeP[node->SM1_idx()] != nullptr)
            {
              node->nodeP[node->SM1_idx()]->busy((bool)frameIn.data[1]); // Information de l'état occupé ou non de SM1
              uint16_t locoAddr = frameIn.data[2] << 8 | frameIn.data[3];
              if (node->SP1_idx() == frameIn.data[0])      // Si le SM1 de ce sat est celui envoyé
                node->nodeP[node->SM1_idx()]->acces(true); // Le SM1 est accessible
              else
                node->nodeP[node->SM1_idx()]->acces(false); // Le SM1 n'est pas accessible
            }
            break;

          case 0xA5:
            node->SP2_acces(frameIn.data[0]); // Enregistrement dans S0 de l'état d'accessibilité de SP2
            node->SP2_busy(frameIn.data[1]);  // ... et enregistrement de l'état d'occupation de SP2
            break;

          // Process de DECOUVERTE
          case 0xC0:                                   // fn : Réception de l'ID d'un satellite et de l'état des liaisons
            if (((Discovery::btnState() & 0x03)) == 1) // Le bouton Sat- est enfoncé
            {
#ifdef DEBUG
              debug.printf("---- fonction 0xC0");
              debug.printf("---- Rec->Reception de l'ID du satellite %d\n", idSatExpediteur);
#endif
              Discovery::ID_satPeriph(idSatExpediteur);
            }
            break;

          case 0xC1:
            if (Discovery::thisID_sat() == (frameIn.id & 0X7F800) >> 11)
            {
              Discovery::retS0_S1(true);
              Discovery::ID_satPeriph(idSatExpediteur);
#ifdef DEBUG
              debug.printf("---- fonction 0xC1");
              debug.printf("----thisID_sat %d\n", (frameIn.id & 0X7F800) >> 11);
              debug.printf("----Discovery::thisID_sat %d\n", Discovery::thisID_sat());
              debug.printf("----Discovery::ID_satPeriph  %d\n", Discovery::ID_satPeriph());
#endif
            }
            break;

            // case 0xC2:                           // Un satellite a envoyé l'état de ses liaisons
            //  if (node->ID() == frameIn.data[1]) // Si l'ID de ce sat est celle du destinataire
            //  {
            //    for (byte i = 0; i < nodePsize; i++)
            //    {
            //      if (node->nodeP[i]->ID() == idSatExpediteur) // Rechrche du sat conerné
            //      {
            //        node->nodeP[i]->liaison[0]->aig = frameIn.data[2];
            //        node->nodeP[i]->liaison[0]->pos = frameIn.data[3];
            //      }
            //    }
            //  }
            // break;

            //   case 0xC3:                           // Un satellite a envoyé l'état de ses liaisons
            //     if (node->ID() == frameIn.data[1]) // Si l'ID de ce sat est celle du destinataire
            //     {
            //       for (byte i = 0; i < nodePsize; i++)
            //       {
            //         if (node->nodeP[i]->ID() == idSatExpediteur) // Rechrche du sat conerné
            //         {
            //           node->nodeP[i]->liaison[0]->aig = frameIn.data[2];
            //           node->nodeP[i]->liaison[0]->pos = frameIn.data[3];
            //           node->nodeP[i]->liaison[1]->aig = frameIn.data[4];
            //           node->nodeP[i]->liaison[1]->pos = frameIn.data[5];
            //         }
            //       }
            //     }
            //     break;

          case 0xC4: // Un satellite demande le masqueAig
            CanMsg::sendMsg(2, node->ID(), *idSatDestinataire, 0xC5, node->masqueAig());
            break;

          case 0xC5: // Un satellite envoie son masqueAig
            node->masqueAig(frameIn.data[0]);
            break;

          case 0xC6: // Un satellite demande le masqueAig du nodeP[p00] de ce node
            CanMsg::sendMsg(2, node->ID(), *idSatDestinataire, 0xC7, node->nodeP[p00]->masqueAig());
            break;

          case 0xC7: // Un satellite envoie le masqueAig de son nodeP[p00]
            node->masqueAigSP2(frameIn.data[0]);
            break;
          }
        }
      }
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
  }
}

/*--------------------------------------
  Envoi
  --------------------------------------*/
void CanMsg::sendMsg(CANMessage &frame)
{
  if (0 == ACAN_ESP32::can.tryToSend(frame))
    debug.printf("Echec envoi message CAN\n");
#ifdef DEBUG
    // else
    //   debug.printf("Envoi fonction 0x%0X\n", frame.data[0]);
#endif
}

auto parseMsg = [](CANMessage &frame, byte priorite, byte idExp, byte idDes, byte fonct) -> CANMessage
{
  frame.id |= priorite << 27; // Priorite 0, 1 ou 2
  frame.id |= idExp << 19;    // ID expediteur
  frame.id |= idDes << 11;    // ID destinataire
  frame.id |= fonct << 3;     // Fonction appelée
  frame.ext = true;
  return frame;
};

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 0;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 1;
  frame.data[0] = data0; // Fonction
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 2;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 3;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 4;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3, byte data4)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 5;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3, byte data4, byte data5)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 6;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  frame.data[5] = data5;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3, byte data4, byte data5, byte data6)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 7;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  frame.data[5] = data5;
  frame.data[6] = data6;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte idExp, byte idDes, byte fonct, byte data0, byte data1, byte data2, byte data3, byte data4, byte data5, byte data6, byte data7)
{
  CANMessage frame;
  frame = parseMsg(frame, priorite, idExp, idDes, fonct);
  frame.len = 8;
  frame.data[0] = data0; // Fonction
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  frame.data[5] = data5;
  frame.data[6] = data6;
  frame.data[7] = data7;
  CanMsg::sendMsg(frame);
}
