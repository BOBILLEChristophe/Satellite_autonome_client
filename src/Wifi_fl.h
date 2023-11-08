/*
  
   Wifi_fl.h


*/


#ifndef __WIFI_FL_H__
#define __WIFI_FL_H__

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include "Config.h"
#include "Settings.h"


struct Fl_Wifi
{
    static void start();
};

#endif /* defined(__WIFI_FL_H__) */
