/*

   WebHandler.cpp


*/

#include "WebHandler.h"

WebHandler::WebHandler() : _server(nullptr), _ws(nullptr) {}

void WebHandler::init(Node *node, uint16_t webPort)
{
  _server = new AsyncWebServer(webPort);
  _ws = new AsyncWebSocket("/ws");
  _ws->onEvent(std::bind(&WebHandler::WsEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));

  WebHandler::route();

  _server->addHandler(_ws);
  _server->begin();

  this->node = node;
}

void WebHandler::loop()
{
  _ws->cleanupClients();
}

void WebHandler::WsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    #ifdef DEBUG
    debug.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
#endif
    WebHandler::notifyClients();
    break;
  case WS_EVT_DISCONNECT:
    #ifdef DEBUG
    debug.printf("WebSocket client #%u disconnected\n", client->id());
#endif
    break;

  case WS_EVT_ERROR:
    //
    break;
  case WS_EVT_PONG:
    //
    break;
  case WS_EVT_DATA:
#ifdef DEBUG
    debug.printf("WebSocket len %d\n", len);
    debug.printf("WebSocket data %s\n\n", data);
#endif
    StaticJsonDocument<1024> doc1; // Memory pool
    DeserializationError error = deserializeJson(doc1, data);

    if (error) // Check for errors in parsing
      debug.println("Parsing failed");

    else
    {
      String message = (char *)data;

      if (message.indexOf("servoSettings") >= 0)
      {
        const char *servoId = doc1["servoSettings"][0];
        uint16_t servoValue = doc1["servoSettings"][1];
        uint8_t servoName = doc1["servoSettings"][2];
#ifdef DEBUG
        debug.printf("servoId %s\n", servoId);
        debug.printf("servoValue %d\n", servoValue);
        debug.printf("servoName %d\n", servoName);
#endif

        if ('0' == servoId[3])
        {
          node->aig[servoName]->posDroit(servoValue);
          node->aig[servoName]->curPos(servoValue);
          node->aig[servoName]->move(servoValue);
        }
        else if ('1' == servoId[3])
        {
          node->aig[servoName]->posDevie(servoValue);
          node->aig[servoName]->curPos(servoValue);
          node->aig[servoName]->move(servoValue);
        }
        else if ('2' == servoId[3])
        {
          // node->aig[servoName]->sSpeed((1 / (float)servoValue) * 10000);
          uint16_t speed = 11000 - (servoValue * 1000);
          node->aig[servoName]->speed(speed);
          #ifdef DEBUG
          debug.printf("speed %d\n", node->aig[servoName]->speed());
          debug.println("------------------------------");
          debug.printf("servo    : %d\n", servoName);
          debug.printf("servoValue : %d\n", servoValue);
          debug.println("------------------------------");
#endif
        }
      }
      // Test de déplacement "en vraie grandeur" de l'aiguille
      if (message.indexOf("servoTest") >= 0)
      {
uint8_t servoName = doc1["servoTest"][0];
#ifdef DEBUG
        debug.println("servoTest");
        debug.printf("servo     : %d\n", servoName);
        debug.printf("gPosDroit : %d\n", node->aig[servoName]->posDroit());
        debug.printf("gPosDevie : %d\n", node->aig[servoName]->posDevie());
#endif
        node->aigRun(servoName);
      }

      if (message.indexOf("wifi_on") >= 0)
      {
#ifdef DEBUG
        debug.println("wifi_on");
#endif
        bool wifi_on = doc1["wifi_on"][0];
        Settings::wifiOn(wifi_on);
      }

      if (message.indexOf("discovery_on") >= 0)
      {
        #ifdef DEBUG
        debug.println("discovery_on");
#endif
        bool discovery_on = doc1["discovery_on"][0];
        Settings::discoveryOn(discovery_on);
      }

      if (message.indexOf("save") >= 0)
      {
        #ifdef DEBUG
        debug.println("save");
#endif
        Settings::writeFile();
      }

      if (message.indexOf("restartEsp") >= 0)
      {
        #ifdef DEBUG
        debug.println("restartEsp");
#endif
        ESP.restart();
      } 
    }
    break;
  }
}

void WebHandler::notifyClients()
{
  //-> https://m1cr0lab-esp32.github.io/remote-control-with-websocket/websocket-and-json/
  //-> https://arduinojson.org/v6/api/jsonobject/

  StaticJsonDocument<1024> doc;

  doc["idNode"] = node->ID();

  // Nœuds
  String index[] = {"p00", "p01", "p10", "p11", "m00", "m01", "m10", "m11"};
  for (byte i = 0; i < 8; i++)
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
      doc["s" + String(i)] = "null";
    else
    {
      doc["s" + String(i)] = "Actif";
      doc["ss" + String(i) + "0"] = node->aig[i]->posDroit();
      doc["ss" + String(i) + "1"] = node->aig[i]->posDevie();
      doc["ss" + String(i) + "2"] = (11000 - node->aig[i]->speed()) / 1000;
    }
  }

  doc["wifi_on"] = Settings::wifiOn();
  doc["discovery_on"] = Settings::discoveryOn();

  String output;
  serializeJson(doc, output);
  _ws->textAll(output);
}

void WebHandler::route()
{
  // Route for root / web page
  _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/index.html", "text/html"); });

  _server->on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/w3.css", "text/css"); });

  _server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/style.css", "text/css"); });

  _server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/script.js", "text/javascript"); });

  _server->on("/settings.json", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/settings.json", "text/json"); });

  _server->on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send(SPIFFS, "/favicon.png", "image/png"); });

  _server->onNotFound([](AsyncWebServerRequest *request)
                      {
    Serial.printf("Not found: %s!\r\n", request->url().c_str());
    request->send(404); });
}