/*

  CanConfig.cpp


*/

#include "CanConfig.h"

void CanConfig::setup(const byte thisID, const bool discovery_on)
{
    debug.printf("[CanConfig %d] : Configure ESP32 CAN\n", __LINE__);
    ACAN_ESP32_Settings settings(CAN_BITRATE);
    settings.mRxPin = CAN_RX;
    settings.mTxPin = CAN_TX;
    uint32_t errorCode;

    if (discovery_on)
    {
           // Si mode discovery, pas de filtre
        errorCode = ACAN_ESP32::can.begin(settings);
        debug.printf("[CanConfig %d] : config without filter\n", __LINE__);
    }
     
    else
    {
        // En fonctionnement, on active le filtre pour ne recevoir que les seuls messages
        // qui concernent ce satellite
        const ACAN_ESP32_Filter filter = ACAN_ESP32_Filter::singleExtendedFilter(
            ACAN_ESP32_Filter::dataAndRemote, thisID << 11, 0x1FF807FF);
        errorCode = ACAN_ESP32::can.begin(settings, filter);
        debug.printf("[CanConfig %d] : config filter\n", __LINE__);
    }

    if (errorCode == 0)
        debug.printf("[CanConfig %d] : configuration OK !\n", __LINE__);
    else
    {
        debug.printf("[CanConfig %d] : configuration error 0x%x\n", __LINE__, errorCode);
        delay(1000);
        return;
    }
}
