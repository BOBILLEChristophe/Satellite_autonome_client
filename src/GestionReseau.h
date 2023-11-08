/*

      GestionReseau.h


*/

#ifndef __GESTION_RESEAU_H__
#define __GESTION_RESEAU_H__

#include <Arduino.h>
#include "CanMsg.h"
#include "Node.h"
#include "Settings.h"

class GestionReseau
{
private:
    static bool isRun;

public:
    GestionReseau() = delete;
    static void setup(Node *);
    static void loop(void *);
    static void run(bool);
};

#endif
