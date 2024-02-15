/*
  
   WebHandler.h


*/

#ifndef __WEBHANDLER_H__
#define __WEBHANDLER_H__

#include "Arduino.h"
#include "Settings.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h> // https://github.com/me-no-dev/ESPAsyncWebServer
#include <SPIFFS.h>
#include <ArduinoJson.h>

#include "Node.h"

class WebHandler
{
  private :
    Node *node;

  protected:
    AsyncWebServer *_server;
    AsyncWebSocket *_ws;
    void WsEvent(AsyncWebSocket*, AsyncWebSocketClient*, AwsEventType, void*, uint8_t*, size_t);

  public:
    WebHandler();
    void init(Node*, uint16_t);
    void loop();
    //void handleWebSocketMessage(void*, uint8_t*, size_t, AsyncWebSocket*, AsyncWebSocketClient*);
    void notifyClients();
    void route();
};



#endif