/*

  CanConfig.cpp


*/

#include "CanConfig.h"

void CanConfig::setup()
{
#ifdef DEBUG
    debug.printf("[CanConfig %d] : Configure ESP32 CAN\n", __LINE__);
#endif
    ACAN_ESP32_Settings settings(CAN_BITRATE);
    settings.mRxPin = CAN_RX;
    settings.mTxPin = CAN_TX;
    uint32_t errorCode;

    errorCode = ACAN_ESP32::can.begin(settings);
#ifdef DEBUG
    debug.printf("[CanConfig %d] : config without filter\n", __LINE__);
#endif

    if (errorCode == 0)
    {
#ifdef DEBUG
        debug.printf("[CanConfig %d] : configuration OK !\n", __LINE__);
#endif
    }
    else
    {
#ifdef DEBUG
        debug.printf("[CanConfig %d] : configuration error 0x%x\n", __LINE__, errorCode);
#endif
        vTaskDelay(pdMS_TO_TICKS(1000));
        return;
    }
}
