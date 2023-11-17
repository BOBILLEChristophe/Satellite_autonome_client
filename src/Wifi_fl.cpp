/*
   
   Wifi_fl.cpp


*/

#include "Wifi_fl.h"

void Fl_Wifi::start()
{

#ifdef WIFI_AP_MODE
    WiFi.softAP(WIFI_SSID, WIFI_PSW);

#ifdef DEBUG
    debug.print("\n");
    debug.print("\n------------WIFI------------");
    debug.print("\nConnected to : ");
    debug.print(WIFI_SSID);
    debug.print("\nIP address :   ");
    debug.print(WiFi.softAPIP());
    debug.print("\n\n");
#endif

#else
    WiFi.begin(Settings::ssid, Settings::password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        debug.print(".");
    }

    debug.print("\n");
    debug.print("\n------------WIFI------------");
    debug.print("\nConnected to : ");
    debug.print(Settings::ssid);
    debug.print("\nIP address :   ");
    debug.print(WiFi.localIP());
    debug.print("\n----------------------------\n\n");

#endif

//     if (!MDNS.begin(MDNS_NAME))
//     {
//         debug.print("Error setting up MDNS responder!\n");
//         while (1)
//             delay(1000);
//     }

// #ifdef DEBUG
//     debug.print("MDNS responder started @ http://");
//     debug.print(MDNS_NAME);
//     debug.print(".local");
//     debug.print("\n\n");
// #endif
}
