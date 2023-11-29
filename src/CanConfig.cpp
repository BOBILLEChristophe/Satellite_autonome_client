/*

  CanConfig.cpp


*/

#include "CanConfig.h"

//void CanConfig::setup(const byte thisID, const bool discovery_on)

void CanConfig::setup(const byte thisIDn)
{
    debug.printf("[CanConfig %d] : Configure ESP32 CAN\n", __LINE__);
    ACAN_ESP32_Settings settings(CAN_BITRATE);
    settings.mRxPin = CAN_RX;
    settings.mTxPin = CAN_TX;
    uint32_t errorCode;

    errorCode = ACAN_ESP32::can.begin(settings);
    debug.printf("[CanConfig %d] : config without filter\n", __LINE__);

    if (errorCode == 0)
        debug.printf("[CanConfig %d] : configuration OK !\n", __LINE__);
    else
    {
        debug.printf("[CanConfig %d] : configuration error 0x%x\n", __LINE__, errorCode);
        vTaskDelay(pdMS_TO_TICKS(1000));
        return;
    }
}
