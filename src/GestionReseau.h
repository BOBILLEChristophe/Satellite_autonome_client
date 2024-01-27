/*

  GestionReseau.h


*/

#ifndef __GESTION_RESEAU_H__
#define __GESTION_RESEAU_H__

#include <Arduino.h>
#include "CanMsg.h"
#include "Node.h"
#include "Settings.h"
#include "SignauxCmd.h"

class GestionReseau
{
private:
    static uint16_t signalValue[2];
public:
    GestionReseau() = delete;
    static void setup(Node *);
    static void IRAM_ATTR loopTask(void *);
    static void signauxTask(void *);
};

#endif
