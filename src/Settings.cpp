/*

   Settings.cpp


*/

#include "Settings.h"

bool Settings::WIFI_ON = true;
bool Settings::DISCOVERY_ON = true;
String Settings::ssid_str;
String Settings::password_str;
char Settings::ssid[30] = {};
char Settings::password[30] = {};
// uint8_t Settings::idNode = NO_ID;
uint8_t Settings::nbLoco = 0;
bool Settings::isMainReady = false;
Node *Settings::node = nullptr;
// uint8_t Settings::gIdNode() { return Settings::idNode; }
uint8_t Settings::gNbLoco() { return nbLoco; }
// void Settings::sIdNode(uint8_t idNode) { Settings::idNode = idNode; }
void Settings::sNbLoco(uint8_t nbLoco) { Settings::nbLoco = nbLoco; }
void Settings::sMainReady(bool val) { Settings::isMainReady = val; }
bool Settings::discoveryOn() { return DISCOVERY_ON; }
void Settings::discoveryOn(bool val) { Settings::DISCOVERY_ON = val; }
bool Settings::wifiOn() { return WIFI_ON; }
void Settings::wifiOn(bool val) { Settings::WIFI_ON = val; }

/*-------------------------------------------------------------
                           begin
--------------------------------------------------------------*/

void Settings::setup(Node *nd)
{
  node = nd;
  readFile();
}

bool Settings::begin()
{
  //--- Test de la présence du Main
  do
  {
    CanMsg::sendMsg(0, node->ID(), 254, 0xB2);
    if (!isMainReady)
    {
#ifdef DEBUG
      debug.printf("Attente de reponse en provenance de la carte Main.\n");
#endif
      delay(1000);
    }
  } while (!isMainReady);

  //--- Identifiant du Node
  while (node->ID() == NO_ID) // L'identifiant n'est pas en mémoire
  {
    //--- Requete identifiant
    CanMsg::sendMsg(0, node->ID(), 254, 0xB4);
    delay(100);
  }
  // node->ID(idNode);

  writeFile();

  //--- Requete nbre locos
  // CanMsg::sendMsg(0, node->ID(), 254, 0xB6);
  // debug.printf("Nombre de locos : %d\n", nbLoco);

  // for (byte i = 0; i < nbLoco; i++)
  // {
  //   debug.printf("Infos loco %d.\n", i);
  //   byte compt = 0;
  //   // while (0 == node->rfid->locoTag[i].address0)
  //   // {
  //   CanMsg::sendMsg(0, node->ID(), 1, 0xB8, i);
  //      delay(1000);
  //   //   if (compt == 10)
  //   //   {
  //   //     debug.println("Impossible de charger la liste des locomotives.");
  //   //     break;
  //   //   }
  //   //   compt++;
  //   // }
  //   compt = 0;
  // }

#ifdef DEBUG
  debug.printf("End settings\n");
  debug.printf("-----------------------------------\n\n");
#endif

  return 0;
} //--- End begin

/*-------------------------------------------------------------
                           readFile
--------------------------------------------------------------*/

void Settings::readFile()
{
  if (!SPIFFS.begin(true))
  {
#ifdef DEBUG
    debug.printf("An Error has occurred while mounting SPIFFS\n\n");
#endif
    return;
  }
  File file = SPIFFS.open("/settings.json", "r");
  if (!file)
  {
#ifdef DEBUG // File not found | le fichier de test n'existe pas
    debug.println("Failed to open settings.json\n\n");
#endif
    return;
  }
  else
  {

#ifdef DEBUG
    debug.printf("\nInformations du fichier \"settings.json\" : \n\n");
#endif

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, file);
    delay(100);
    if (error)
#ifdef DEBUG
      debug.printf("Failed to read file, using default configuration\n\n");
#endif
    else
    {
      // ---
      node->ID(doc["idNode"] | NO_ID);
#ifdef DEBUG
      debug.printf("- ID node : %d\n", node->ID());
#endif
      Discovery::comptAig(doc["comptAig"]);
      node->masqueAig(doc["masqueAig"]);
      WIFI_ON = doc["wifi_on"];
      DISCOVERY_ON = doc["discovery_on"];
      ssid_str = doc["ssid"].as<String>();
      password_str = doc["password"].as<String>();
      strcpy(ssid, ssid_str.c_str());
      strcpy(password, password_str.c_str());

      // Nœuds
      const char *index[] = {"p00", "p01", "p10", "p11", "m00", "m01", "m10", "m11"};
      for (byte i = 0; i < nodePsize; i++)
      {
        if (doc[index[i]] != "null")
        {
          if (node->nodeP[i] == nullptr)
            node->nodeP[i] = new NodePeriph;
          node->nodeP[i]->ID(doc[index[i]]);
#ifdef DEBUG
          debug.printf("- node->nodeP[%s]->id : %d\n", index[i], node->nodeP[i]->ID());
#endif
        }
#ifdef DEBUG
        else
          debug.printf("- node->nodeP[%s]->id : NULL\n", index[i]);
#endif
      }
      debug.printf("---------------------------------\n");

      // Aiguilles
      for (byte i = 0; i < aigSize; i++)
      {
        if (doc["aig" + String(i)] != "null")
        {
          // node->aig[i] = new Aig;
          node->aig[i]->ID(doc["aig" + String(i) + "id"]);
          node->aig[i]->posDroit(doc["aig" + String(i) + "posDroit"]);
          node->aig[i]->posDevie(doc["aig" + String(i) + "posDevie"]);
          node->aig[i]->curPos(doc["aig" + String(i) + "curPos"]);
          node->aig[i]->speed(doc["aig" + String(i) + "speed"]);
          node->aig[i]->signalPin(doc["aig" + String(i) + "Pin"]);
          node->aig[i]->setup();
          debug.printf("- Creation de l'aiguille %d\n", i);
        }
      }
#ifdef DEBUG
      debug.printf("---------------------------------\n");
#endif
    }
    file.close();
    // SPIFFS.end();
  }
} //--- End readFile

/*-------------------------------------------------------------
                           writeFile
--------------------------------------------------------------*/

void Settings::writeFile()
{
  if (!SPIFFS.begin(true))
  {
#ifdef DEBUG
    debug.println("An Error has occurred while mounting SPIFFS\n\n");
#endif
    return;
  }
  File file = SPIFFS.open("/settings.json", "w");
  if (!file)
  {
#ifdef DEBUG
    debug.println("Failed to open settings.json\n\n");
#endif
    return;
  }
  else
  {
    DynamicJsonDocument doc(2048);

    doc["idNode"] = node->ID();
    doc["comptAig"] = Discovery::comptAig();
    doc["masqueAig"] = node->masqueAig();
    doc["wifi_on"] = WIFI_ON;
    doc["discovery_on"] = DISCOVERY_ON;
    doc["ssid"] = ssid;
    doc["password"] = password;

    // Nœuds
    const String index[] = {"p00", "p01", "p10", "p11", "m00", "m01", "m10", "m11"};
    for (byte i = 0; i < nodePsize; i++)
    {
      if (node->nodeP[i] == nullptr)
        doc[index[i]] = "null";
      else
        doc[index[i]] = node->nodeP[i]->ID();
    }

    // Aiguilles
    for (byte i = 0; i < aigSize; i++)
    {
      if (node->aig[i] == nullptr)
        doc["aig" + String(i)] = "null";
      else
      {
        doc["aig" + String(i) + "id"] = node->aig[i]->ID();
        doc["aig" + String(i) + "posDroit"] = node->aig[i]->posDroit();
        doc["aig" + String(i) + "posDevie"] = node->aig[i]->posDevie();
        doc["aig" + String(i) + "curPos"] = node->aig[i]->curPos();
        doc["aig" + String(i) + "speed"] = node->aig[i]->speed();
        doc["aig" + String(i) + "Pin"] = node->aig[i]->signalPin();
      }
    }

    String output;
    serializeJson(doc, output);
    file.print(output);
    file.close();
    // SPIFFS.end();
#ifdef DEBUG
    debug.print("Sauvegarde des datas en FLASH\n");
#endif
  }
} //--- End writeFile
