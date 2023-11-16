/*
  
   Settings.h


*/

#ifndef __SETTINGS__
#define __SETTINGS__

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <FS.h>
#include "CanMsg.h"
#include "CanConfig.h"
#include "Config.h"
#include "Node.h"

class Settings
{
private:

  //static uint8_t nbLoco;
  static bool isMainReady;
  static String ssid_str;
  static String password_str;
  static bool WIFI_ON;
  static bool DISCOVERY_ON;
  static Node *node;

public:
  static char ssid[30];
  static char password[30];
  Settings() = delete;
  static void setup(Node *);
  static bool begin();
  static void writeFile();
  static void readFile();
  static uint8_t gNbLoco();
  static void sNbLoco(const uint8_t);
  static void sMainReady(bool);
  static bool discoveryOn();
  static void discoveryOn(bool);
  static bool wifiOn();
  static void wifiOn(bool);
};

#endif
