/*

  CanConfig.cpp


*/

#include "CanConfig.h"

void CanConfig::setup(const byte thisID, const bool discovery_on)
{
    debug.println("Configure ESP32 CAN");
    ACAN_ESP32_Settings settings(CAN_BITRATE);
    settings.mRxPin = CAN_RX;
    settings.mTxPin = CAN_TX;
    uint32_t errorCode;

    if (discovery_on)
        // Si mode discovery, pas de filtre
        errorCode = ACAN_ESP32::can.begin(settings);
    else
    {
        // En fonctionnement, on active le filtre pour ne recevoir que les seuls messages
        // qui concernent ce satellite
        const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::singleExtendedFilter(
            ACAN_ESP32_Filter::dataAndRemote, thisID << 11, 0x1FF807FF);
        errorCode = ACAN_ESP32::can.begin(settings, filter);
        debug.print("Can config filter\n");
    }

    if (errorCode == 0)
        debug.println("Can configuration OK !\n");
    else
    {
        debug.printf("Can configuration error 0x%x\n", errorCode);
        delay(1000);
        return;
    }
}
