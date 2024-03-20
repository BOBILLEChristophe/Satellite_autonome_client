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
bool Settings::isMainReady = false;
Node *Settings::node = nullptr;
void Settings::sMainReady(bool val) { Settings::isMainReady = val; }
bool Settings::discoveryOn() { return DISCOVERY_ON; }
void Settings::discoveryOn(bool val) { Settings::DISCOVERY_ON = val; }
bool Settings::wifiOn() { return WIFI_ON; }
void Settings::wifiOn(bool val) { Settings::WIFI_ON = val; }

/*-------------------------------------------------------------
                           setup
--------------------------------------------------------------*/

void Settings::setup(Node *nd)
{
  node = nd;
  if (!SPIFFS.begin(true))
  {
    Serial.printf("[Settings %d] : An Error has occurred while mounting SPIFFS\n\n", __LINE__);
    return;
  }
  else
    Serial.printf("[Settings %d] : SPIFFS ok mounting\n", __LINE__);
  readFile();
}

/*-------------------------------------------------------------
                           begin
--------------------------------------------------------------*/

bool Settings::begin()
{
  //--- Test de la présence de la carte Main

  Serial.printf("[Settings %d] : Attente de reponse en provenance de la carte Main.\n", __LINE__);
  do
  {
    CanMsg::sendMsg(0, 0xB2, node->ID(), 254, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
  } while (!isMainReady );

  //Identifiant du Node
  if (node->ID() == NO_ID)
    Serial.printf("[Settings %d] : Le satellite ne possede pas d'identifiant.\n", __LINE__);

  while (node->ID() == NO_ID) // L'identifiant n'est pas en mémoire
  {
    //--- Requete identifiant
    CanMsg::sendMsg(0, 0xB4, node->ID(), 254, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
  }

  //writeFile(); // Sauvegarde de donnees en flash

  Serial.printf("[Settings %d] : End settings\n", __LINE__);
  Serial.printf("-----------------------------------\n\n");

  return 0;
} //--- End begin

/*-------------------------------------------------------------
                           readFile
--------------------------------------------------------------*/

void Settings::readFile()
{
  File file = SPIFFS.open("/settings.json", "r");
  if (!file)
  {
#ifdef DEBUG
    debug.printf("[Settings %d] : Failed to open settings.json\n\n", __LINE__);
#endif
    return;
  }
  else
  {
#ifdef DEBUG
    debug.printf("\nInformations du fichier \"settings.json\" : \n\n");
#endif
    DynamicJsonDocument doc(4 * 1024);
    DeserializationError error = deserializeJson(doc, file);
    vTaskDelay(pdMS_TO_TICKS(100));
    if (error)
    {
#ifdef DEBUG
      debug.printf("[Settings %d] Failed to read file, using default configuration\n\n", __LINE__);
#endif
    }
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
      node->maxSpeed(doc["maxSpeed"]);
      node->sensMarche(doc["sensMarche"]);

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
#ifdef DEBUG
      debug.printf("---------------------------------\n");
#endif

      // Aiguilles
      for (byte i = 0; i < aigSize; i++)
      {
        // debug.printf("valeur de aig %d : %s%c\n", i, doc["aig" + String(i)]);
        if (doc["aig" + String(i)] != "null")
        {
          if (node->aig[i] == nullptr)
            node->aig[i] = new Aig;
          node->aig[i]->ID(doc["aig" + String(i) + "id"]);
          node->aig[i]->posDroit(doc["aig" + String(i) + "posDroit"]);
          node->aig[i]->posDevie(doc["aig" + String(i) + "posDevie"]);
          node->aig[i]->speed(doc["aig" + String(i) + "speed"]);
          node->aig[i]->pin(doc["aig" + String(i) + "pin"]);
          node->aig[i]->setup();
#ifdef DEBUG
          debug.printf("- Creation de l'aiguille %d\n", i);
#endif
        }
      }
#ifdef DEBUG
      debug.printf("---------------------------------\n");
#endif

      // Signaux
      for (byte i = 0; i < signalSize; i++)
      {
        if (doc["sign" + String(i)] != "null")
        {
          if (node->signal[i] == nullptr)
            node->signal[i] = new Signal;
          node->signal[i]->type(doc["sign" + String(i) + "type"]);
          node->signal[i]->position(doc["sign" + String(i) + "position"]);
#ifdef DEBUG
          debug.printf("- Creation du signal %d\n", i);
#endif
        }
      }
#ifdef DEBUG
      debug.printf("---------------------------------\n");
#endif
     }
    file.close();
  }
} //--- End readFile

/*-------------------------------------------------------------
                           writeFile
--------------------------------------------------------------*/

void Settings::writeFile()
{
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
    DynamicJsonDocument doc(4 * 1024);

    doc["idNode"] = node->ID();
    doc["comptAig"] = Discovery::comptAig();
    doc["masqueAig"] = node->masqueAig();
    doc["wifi_on"] = WIFI_ON;
    doc["discovery_on"] = DISCOVERY_ON;
    doc["ssid"] = ssid;
    doc["password"] = password;
    doc["maxSpeed"] = node->maxSpeed();
    doc["sensMarche"] = node->sensMarche();

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
        doc["aig" + String(i) + "speed"] = node->aig[i]->speed();
        doc["aig" + String(i) + "pin"] = node->aig[i]->pin();
      }
    }

    // Signaux
    for (byte i = 0; i < signalSize; i++)
    {
      if (node->signal[i] == nullptr)
        doc["sign" + String(i)] = "null";
      else
      {
        doc["sign" + String(i) + "type"] = node->signal[i]->type();
        doc["sign" + String(i) + "position"] = node->signal[i]->position();
      }
    }

    String output;
    serializeJson(doc, output);
    file.print(output);
    file.close();
#ifdef DEBUG
    debug.print("Sauvegarde des datas en FLASH\n");
#endif
  }
} //--- End writeFile
