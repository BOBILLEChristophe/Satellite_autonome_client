/*

   Wifi_fl.cpp


*/

#include "Wifi_fl.h"

void Fl_Wifi::start()
{

#ifdef WIFI_AP_MODE
    WiFi.softAP(WIFI_SSID, WIFI_PSW);

    Serial.print("\n");
    Serial.print("\n------------WIFI------------");
    Serial.print("\nConnected to : ");
    Serial.print(WIFI_SSID);
    Serial.print("\nIP address :   ");
    Serial.print(WiFi.softAPIP());
    Serial.print("\n\n");

#else
    WiFi.begin(Settings::ssid, Settings::password);
    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        Serial.print(".");
    }

    Serial.print("\n");
    Serial.print("\n------------WIFI------------");
    Serial.print("\nConnected to : ");
    Serial.print(Settings::ssid);
    Serial.print("\nIP address :   ");
    Serial.print(WiFi.localIP());
    Serial.print("\n----------------------------\n\n");

#endif

    //     if (!MDNS.begin(MDNS_NAME))
    //     {
    //         debug.print("Error setting up MDNS responder!\n");
    //         while (1)
    //             vTaskDelay(pdMS_TO_TICKS(1000));
    //     }

    // #ifdef DEBUG
    //     debug.print("MDNS responder started @ http://");
    //     debug.print(MDNS_NAME);
    //     debug.print(".local");
    //     debug.print("\n\n");
    // #endif
}
