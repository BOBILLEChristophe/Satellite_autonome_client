/*

  Discovery.cpp


*/
#include "Discovery.h"

//    Node p00;     // Le satellite qui est dans le sens horaire (sans aiguille ou aiguilles 0 et 1 droites)
//    Node p01;     // Le satellite qui est dans le sens horaire (aiguille 0 déviée - si il y en a une, aiguille 2 droite)
//    Node p10;     // Le satellite qui est dans le sens horaire (aiguille 0 droite - aiguille 1 déviée)
//    Node p11;     // Le satellite qui est dans le sens horaire (aiguille 0 déviée - aiguille 2 déviée)
//    Node m00;     // Le satellite qui est dans le sens antihoraire (sans aiguille ou aiguilles 3 et 4 droites)
//    Node m01;     // Le satellite qui est dans le sens antihoraire (aiguille 3 déviée et, si il y en a une, aiguille 5 droite)
//    Node m10;     // Le satellite qui est dans le sens antihoraire (aiguille 3 droite - aiguille 4 déviée)
//    Node m11;     // Le satellite qui est dans le sens antihoraire (aiguille 3 déviée - aiguille 5 déviée)

byte Discovery::m_switchAig{0};
byte Discovery::m_btnState{0};
bool Discovery::m_toggleLed{LOW};
byte Discovery::m_ID_satPeriph{NO_ID};
byte Discovery::m_thisID_sat{NO_ID};
bool Discovery::m_retS0_S1{false};
byte Discovery::m_comptAig(0);

void Discovery::retS0_S1(bool val) { m_retS0_S1 = val; }
bool Discovery::retS0_S1() { return m_retS0_S1; }
void Discovery::ID_satPeriph(byte val) { m_ID_satPeriph = val; }
byte Discovery::ID_satPeriph() { return m_ID_satPeriph; }
void Discovery::thisID_sat(byte val) { m_thisID_sat = val; }
byte Discovery::thisID_sat() { return m_thisID_sat; }
void Discovery::comptAig(byte val) { m_comptAig = val; }
byte Discovery::comptAig() { return m_comptAig; }
void Discovery::btnState(byte val) { m_btnState = val; }
byte Discovery::btnState() { return m_btnState; }
const byte Discovery::m_aigPin[] = {AIG_PIN_SIGNAL_0, AIG_PIN_SIGNAL_1, AIG_PIN_SIGNAL_2, AIG_PIN_SIGNAL_3};
const gpio_num_t Discovery::m_pinIn[] = {BTN_SAT_MOINS, BTN_SAT_PLUS, INTER_DEV_1, INTER_DEV_2}; // 35 -> btn-, 36 -> btn+, 34 -> switch 1, 39 -> switch 2
const gpio_num_t Discovery::m_pinLed = {LED_PIN_DISCOV};
Node *Discovery::node = nullptr;

void Discovery::begin(Node *nd)
{
  node = nd;
  //--- Initialisation des boutons et switches
  for (byte i = 0; i < 4; i++)
    pinMode(m_pinIn[i], INPUT);
  pinMode(m_pinLed, OUTPUT);

  m_thisID_sat = node->ID();

  TaskHandle_t discoveryProcessHandle = nullptr;
  xTaskCreate(process, "Process", 4 * 1024, (void *)node, 2, NULL);
  xTaskCreate(createCiblesSignaux, "createCiblesSignaux", 4 * 1024, (void *)node, 2, NULL);
}

void Discovery::process(void *pvParameters)
{
  bool btnMoinsState{LOW};
  Node *node;
  node = (Node *)pvParameters;

  TickType_t xLastWakeTime;
  xLastWakeTime = xTaskGetTickCount();

  for (;;)
  {
    for (byte i = 0; i < 4; i++)
    { // Lecture de l'état des BP et des switches
      if (!digitalRead(m_pinIn[i]))
        m_btnState |= (1 << i); // m_btnState est une variable qui stocke l'etat des boutons et switches
                                // sous forme binaire
      else
        m_btnState &= ~(1 << i);
    }

    switch (m_btnState & 0x03) // Concerne les seuls BP
    {
    case 0x02: // Btn+ actif
      m_toggleLed = !m_toggleLed;
      // Envoi sur le bus CAN l'ID du satellite S1
      CanMsg::sendMsg(0, m_thisID_sat, NO_ID, 0xC0);

      // Retour de S0 vers S1
      if (m_retS0_S1) // La variable retS0_S1 est MAJ dans CanMsg.cpp case 0xC1
      {
        if (node->nodeP[(m_btnState >> 2) + 4] == nullptr)
          node->nodeP[(m_btnState >> 2) + 4] = new NodePeriph; // Création d'un nœud périphérique relié à ce satellite

        node->nodeP[(m_btnState >> 2) + 4]->ID(m_ID_satPeriph); // Attribution à ce noeud de l'ID reçu
        m_retS0_S1 = false;
        m_toggleLed = HIGH;
      }
      break;

    case 0x01: // Btn- actif
#ifdef DEBUG
      debug.print("Btn- actif, switch : ");
      debug.println((m_btnState & 0x0C) >> 2);
#endif
      if (btnMoinsState == LOW)
        m_ID_satPeriph = NO_ID;
      btnMoinsState = HIGH;
      if (m_ID_satPeriph == NO_ID)
        m_toggleLed = !m_toggleLed;
      else
      {
        m_toggleLed = HIGH;
        // Reception de données
#ifdef DEBUG
        debug.printf("btnState >> 2 %d\n", m_btnState >> 2);
#endif
        if (node->nodeP[m_btnState >> 2] == nullptr)
          node->nodeP[m_btnState >> 2] = new NodePeriph;

        node->nodeP[m_btnState >> 2]->ID(m_ID_satPeriph); /* Le node peripherique prend la valeur envoyée par S1
                                                        récupérée par la fonction 0xC0 de CanMsg::receiveMsg */
        // S0 envoie à son tour son ID à S1
        CanMsg::sendMsg(0, m_thisID_sat, m_ID_satPeriph, 0xC1);
      }
      break;
    }

    if (m_btnState & (1 << 0) || m_btnState & (1 << 1))
      createAiguilles();

    // --- Gestion de la led et du bouton
    if (!(m_btnState & 0x03)) // btn- et btn+ relachés
    {
      m_toggleLed = LOW;
      btnMoinsState = LOW;
    }
    // --- Reset : delete tous les noeuds periph, les aiguilles et les signaux
    else if (3 == (m_btnState & 0x03)) // btn- et btn+ appuyés simultanément
    {
      for (byte i = 0; i < nodePsize; i++)
      {
        if (node->nodeP[i] != nullptr)
        {
          delete node->nodeP[i];
          node->nodeP[i] = nullptr;
        }
      }

      for (byte i = 0; i < aigSize; i++)
      {
        if (node->aig[i] != nullptr)
        {
          delete node->aig[i];
          node->aig[i] = nullptr;
        }

        if (node->signal[i] != nullptr)
        {
          delete node->signal[i];
          node->signal[i] = nullptr;
        }
      }
      m_comptAig = 0;
    }
    digitalWrite(m_pinLed, m_toggleLed);

    vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100)); // toutes les x ms
  }
}

void Discovery::createAiguilles() // Création des aiguilles
{
  node->masqueAig(0x00);
  m_comptAig = 0;
  for (Aig *el : node->aig)
    el = nullptr;

  auto createAig = [&](uint8_t index)
  {
    if (m_comptAig < 4)
    {
      if (node->aig[index] == nullptr)
        node->aig[index] = new Aig;

      node->aig[index]->ID(index);
      node->aig[index]->signalPin(m_aigPin[m_comptAig]);
      node->aig[index]->setup();
      node->m_masqueAig &= (1 << index);
#ifdef DEBUG
      debug.printf("Creation de l'aiguille %d\n", index);
#endif
      m_comptAig++;
    }
  };

  const byte aigConditions[aigSize][2] = {
      {p00, p01}, {p00, p10}, {p01, p11}, {m00, m01}, {m00, m10}, {m01, m11}};

  for (uint8_t i = 0; i < aigSize; i++)
  {
    auto condition = aigConditions[i];
    if (node->nodeP[condition[0]] != nullptr && node->nodeP[condition[1]] != nullptr)
      createAig(i);
  }
}

void Discovery::createCiblesSignaux(void *pvParameters)
{
  Node *node;
  node = (Node *)pvParameters;

  for (;;)
  {

    // debug.printf("[Discovery %d] : Discovery:: createCiblesSignaux() running\n", __LINE__);

    //   //******************** MAJ des masqueAig **********************************
    // Pour chaque noeud périphérique on demande la valeur de son masqueAig
    for (byte i = 0; i < nodePsize; i++)
    {

      if (node->nodeP[i] != nullptr)
      {
        CanMsg::sendMsg(2, node->ID(), node->nodeP[i]->ID(), 0xC4);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        // debug.printf("[Discovery %d] : Reception du masqueAig de sat %d = %d \n", __LINE__, node->nodeP[i]->ID(), node->nodeP[i]->masqueAig());
      }
    }
    //******************* Cibles de signalisation ******************************
    // Sens horaire -> aig0 est droit ou il n'y a pas d'aig0
    // Signal avant l'aiguille 0 ou en sortie de canton si pas d'aiguille 0

    auto createSign = [node](uint8_t idxSig)
    {
      byte typeCible = 0;
      if (node->signal[idxSig] == nullptr)
        node->signal[idxSig] = new Signal;
      node->signal[idxSig]->ID(idxSig);

      // node->signal[idxSig]->type(0);                             // typeCible = 0 (qui est le feu par defaut)
      if (node->masqueAig() & (1 << idxSig)) // Une aiguille existe
        // node->signal[idxSig]->type(3);                           // RRalentissement (avec Carré) - > pas besoin d'aller plus loin
        typeCible = 3;
      else // Pas d'aiguille
      {
        if (node->nodeP[idxSig]->masqueAig() & (1 << (idxSig + 3))) // Il faut chercher si l'aiguille 3 de SP1 existe
          // node->signal[idxSig]->type(1);                         // Si oui -> la cible est (au moins) un carré
          typeCible = 1;
        if (node->nodeP[idxSig]->masqueAig() & (1 << idxSig)) // Il faut chercher si l'aiguille 0 de SP1 (SM1) existe
          // node->signal[idxSig]->type(2);                         // Ralentissement (avec Carré)
          typeCible = 2; // L'aiguille 0 de SP1 (SM1) n'existe pas
        else
        {                                                         // Il faut savoir s'il y a une aiguille 3 à SP2
          CanMsg::sendMsg(0, node->nodeP[idxSig]->ID(), 0, 0xC6); // On adresse une demande à SP2
          vTaskDelay(pdMS_TO_TICKS(100));                                             // On attend 1/10 de sec
          if (node->masqueAigSP2() & (1 << idxSig))               // Si oui
            // node->signal[idxSig]->type(2);                     // Ralentissement (avec Carré)
            typeCible = 2;
        }
      }
      node->signal[idxSig]->type(typeCible);
      debug.printf("[Discovery %d] : Le type de cible pour la sortie %d = %d \n", __LINE__, idxSig, node->signal[idxSig]->type());
    };

    for (byte i = 0; i < aigSize; i++)
    {
      // if (node->m_masqueAig & (1 << i))
      if (node->nodeP[i] != nullptr)
        createSign(i);
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    //**************************************************************************
  }
}
