/*

  CanMsg.cpp


*/

#include "CanMsg.h"

void CanMsg::setup(Node *node)
{
#ifdef DEBUG
  debug.printf("[CanMsg %d] : setup\n", __LINE__);
#endif
  TaskHandle_t canReceiveHandle = NULL;
  xTaskCreatePinnedToCore(canReceiveMsg, "CanReceiveMsg", 2 * 1024, (void *)node, 6, &canReceiveHandle, 0); // Création de la tâches pour le traitement
#ifdef TEST_MEMORY_TASK
  xTaskCreate(testMemory, "TestMemory", 2 * 1024, (void *)canReceiveHandle, 2, NULL); // Création de la tâches pour le traitement
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
  Reception CAN
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
      const byte commande = (frameIn.id & 0x7F80000) >> 19;
      const byte idSatExpediteur = (frameIn.id & 0x7F800) >> 11; // ID du satellite qui envoie
      const bool response = (frameIn.id & 0x08) >> 3;
#ifdef DEBUG
      debug.printf("\n[CanMsg %d]------ Expediteur %d : commande 0x%0X\n", __LINE__, idSatExpediteur, commande);
#endif
      if (frameIn.rtr) // Remote frame
      {
#ifdef DEBUG
        debug.printf("[CanMsg %d Frame de remote \n", __LINE__);
#endif
        switch (commande)
        {
        case 0x0F:
          ACAN_ESP32::can.tryToSend(frameIn);
          break;
        }
      }
      else
      {
        switch (commande) // commande appelee
        {
        case 0xB3: // fn : Reponse à demande de test du bus CAN
          if (frameIn.data[0])
            Settings::sMainReady(true);
          break;
        case 0xB5: // fn : Reponse à demande d'identifiant (0xB4)
          if (node->ID() == NO_ID)
            node->ID(frameIn.data[0]);
          break;
        case 0xBD: // Activation  - desactivation du WiFi
          Settings::wifiOn(frameIn.data[0]);
          break;
        case 0xBE: // Activation  - desactivation du mode Discovery
          if (frameIn.data[0])
            Discovery::stopProcess(true);
          else
            Settings::discoveryOn(true);
          break;
        case 0xBF: // fn : Enreistrement des données en mémoire flash
#ifdef SAUV_BY_MAIN
#ifdef DEBUG
          debug.printf("------ Rec->sauvegarde distante\n");
#endif
          Settings::writeFile();
#else
          debug.printf("Sauvegarde automatique desactivee.\n");
#endif
          break;
        case 0xC0: // fn : Réception de l'ID d'un satellite
          Discovery::ID_satPeriph(idSatExpediteur);
          break;

        case 0xC1:
          /*****************************************************************************************************
           * reception periodique des data envoyees par les sat pendant le processus de decouverte
           ******************************************************************************************************/

          // debug.printf("[CanMsg %d] : commande 0xC1, ID exped %d \n", __LINE__, idSatExpediteur);

          for (auto el : node->nodeP)
          {
            if (el != nullptr)
            {
              if (idSatExpediteur == el->ID()) // Si l'expediteur est un SP1 ou un SM1
                el->masqueAig(frameIn.data[0]);
            }
          }
          break;
        case 0xE0:
          /*****************************************************************************************************
           * reception periodique des data envoyees par les sat en exploitation (GestionReseau.cpp ligne 42)
           ******************************************************************************************************/

          //  debug.printf("[CanMsg %d] commande 0xE0\n", __LINE__);
          //  debug.printf("[CanMsg %d] idSatExpediteur : %d\n", __LINE__, idSatExpediteur);
          //  debug.printf("[CanMsg %d] nodeP[0]->ID() : %d\n", __LINE__, node->nodeP[0]->ID());
          //  debug.printf("[CanMsg %d] node->nodeP[node->SP1_idx()]->ID() : %d\n", __LINE__, node->nodeP[node->SP1_idx()]->ID());
          //  debug.printf("[CanMsg %d] node->tabInvers[idSatExpediteur] : %d\n", __LINE__, node->tabInvers[idSatExpediteur]);
          //  debug.printf("[CanMsg %d] SM1 ID : %d\n", __LINE__, frameIn.data[1]);
          //  debug.printf("[CanMsg %d] SP1 ID : %d\n", __LINE__, frameIn.data[2]);

          if (node->nodeP[node->SP1_idx()] != nullptr)
          {
            if (idSatExpediteur == node->nodeP[node->SP1_idx()]->ID()) // Si l'expediteur est SP1
            {
              // L'ID SP1 que l'on recoit est il l'ID de ce sat => ? le sat est accessible : le sat n'est pas accessible
              if (node->ID() == frameIn.data[2])
              {
                node->nodeP[node->SP1_idx()]->acces(true);
                node->SP2_acces(frameIn.data[4]);
                node->SP2_busy(frameIn.data[5]);
              }
              else
                node->nodeP[node->SP1_idx()]->acces(false);
              node->nodeP[node->SP1_idx()]->locoAddr((frameIn.data[0] << 8) + frameIn.data[1]);
              if (node->nodeP[node->SP1_idx()]->locoAddr())
                node->nodeP[node->SP1_idx()]->busy(true);
              else
                node->nodeP[node->SP1_idx()]->busy(false);
              // debug.printf("[CanMsg %d] node->nodeP[node->SP1_idx()]->acces() : %d\n", __LINE__, node->nodeP[node->SP1_idx()]->acces());
              // debug.printf("[CanMsg %d] node->nodeP[node->SP1_idx()]->busy() : %d\n", __LINE__, node->nodeP[node->SP1_idx()]->busy());
            }
          }

          if (node->nodeP[node->SM1_idx()] != nullptr)
          {
            if (idSatExpediteur == node->nodeP[node->SM1_idx()]->ID()) // Si l'expediteur est SM1
            {
              // L'ID SM1 que l'on recoit est il l'ID de ce sat => ? le sat est accessible : le sat n'est pas accessible
              // debug.printf("[CanMsg %d] node->ID() : %d\n", __LINE__, node->ID());
              if (node->ID() == frameIn.data[3])
              {
                node->nodeP[node->SM1_idx()]->acces(true);
                node->SM2_acces(frameIn.data[6]);
                node->SM2_busy(frameIn.data[7]);
              }
              else
                node->nodeP[node->SM1_idx()]->acces(false);
              node->nodeP[node->SM1_idx()]->locoAddr((frameIn.data[0] << 8) + frameIn.data[1]);
              if (node->nodeP[node->SM1_idx()]->locoAddr())
                node->nodeP[node->SM1_idx()]->busy(true);
              else
                node->nodeP[node->SM1_idx()]->busy(false);
              // debug.printf("[CanMsg %d] node->nodeP[node->SM1_idx()]->acces() : %d\n", __LINE__, node->nodeP[node->SM1_idx()]->acces());
              // debug.printf("[CanMsg %d] node->nodeP[node->SM1_idx()]->busy() : %d\n", __LINE__, node->nodeP[node->SM1_idx()]->busy());
            }
          }
          break;
        }
      }
    }
    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(10));
  }
}

/*--------------------------------------
  Envoi CAN
  --------------------------------------*/

void CanMsg::sendMsg(CANMessage &frame)
{
#ifdef DEBUG
  if (0 == ACAN_ESP32::can.tryToSend(frame))
    debug.printf("Echec envoi message CAN\n");
  else
    debug.printf("Envoi commande 0x%0X\n", (frame.id & 0x7F80000) >> 19);
#else
  ACAN_ESP32::can.tryToSend(frame);
#endif
}

auto formatMsg = [](CANMessage &frame, byte priorite, byte cmde, byte idExp, byte idDes, bool resp) -> CANMessage
{
  frame.id |= priorite << 27; // Priorite 0, 1 ou 2
  frame.id |= cmde << 19;     // commande appelée
  frame.id |= idExp << 11;    // ID expediteur
  frame.id |= idDes << 3;     // ID destinataire
  frame.id |= resp << 2;      // Response
  frame.ext = true;
  return frame;
};

void CanMsg::sendMsg(byte priorite, byte cmde, byte idExp, byte idDes, byte resp)
{
  CANMessage frame;
  frame = formatMsg(frame, priorite, cmde, idExp, idDes, resp);
  frame.len = 0;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte cmde, byte idExp, byte idDes, byte resp, byte data0)
{
  CANMessage frame;
  frame = formatMsg(frame, priorite, cmde, idExp, idDes, resp);
  frame.len = 1;
  frame.data[0] = data0;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte cmde, byte idExp, byte idDes, byte resp, byte data0, byte data1)
{
  CANMessage frame;
  frame = formatMsg(frame, priorite, cmde, idExp, idDes, resp);
  frame.len = 2;
  frame.data[0] = data0;
  frame.data[1] = data1;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte cmde, byte idExp, byte idDes, byte resp, byte data0, byte data1, byte data2)
{
  CANMessage frame;
  frame = formatMsg(frame, priorite, cmde, idExp, idDes, resp);
  frame.len = 3;
  frame.data[0] = data0;
  frame.data[1] = data1;
  frame.data[2] = data2;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte cmde, byte idExp, byte idDes, byte resp, byte data0, byte data1, byte data2, byte data3)
{
  CANMessage frame;
  frame = formatMsg(frame, priorite, cmde, idExp, idDes, resp);
  frame.len = 4;
  frame.data[0] = data0;
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte cmde, byte idExp, byte idDes, byte resp, byte data0, byte data1, byte data2, byte data3, byte data4)
{
  CANMessage frame;
  frame = formatMsg(frame, priorite, cmde, idExp, idDes, resp);
  frame.len = 5;
  frame.data[0] = data0;
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte cmde, byte idExp, byte idDes, byte resp, byte data0, byte data1, byte data2, byte data3, byte data4, byte data5)
{
  CANMessage frame;
  frame = formatMsg(frame, priorite, cmde, idExp, idDes, resp);
  frame.len = 6;
  frame.data[0] = data0;
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  frame.data[5] = data5;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte cmde, byte idExp, byte idDes, byte resp, byte data0, byte data1, byte data2, byte data3, byte data4, byte data5, byte data6)
{
  CANMessage frame;
  frame = formatMsg(frame, priorite, cmde, idExp, idDes, resp);
  frame.len = 7;
  frame.data[0] = data0;
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  frame.data[5] = data5;
  frame.data[6] = data6;
  CanMsg::sendMsg(frame);
}

void CanMsg::sendMsg(byte priorite, byte cmde, byte idExp, byte idDes, byte resp, byte data0, byte data1, byte data2, byte data3, byte data4, byte data5, byte data6, byte data7)
{
  CANMessage frame;
  frame = formatMsg(frame, priorite, cmde, idExp, idDes, resp);
  frame.len = 8;
  frame.data[0] = data0;
  frame.data[1] = data1;
  frame.data[2] = data2;
  frame.data[3] = data3;
  frame.data[4] = data4;
  frame.data[5] = data5;
  frame.data[6] = data6;
  frame.data[7] = data7;
  CanMsg::sendMsg(frame);
}
