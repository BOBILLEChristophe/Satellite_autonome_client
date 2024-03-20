/*

  GestionReseau.cpp


*/

#include "GestionReseau.h"

uint16_t GestionReseau::signalValue[2] = {0};

void GestionReseau::setup(Node *node)
{
    xTaskCreatePinnedToCore(signauxTask, "SignauxTask", 2 * 1024, (void *)node, 8, NULL, 0); // Création de la tâches pour les signaux
    xTaskCreatePinnedToCore(loopTask, "LoopTask", 4 * 1024, (void *)node, 10, NULL, 0);      // Création de la tâches pour le traitement
    // tskIDLE_PRIORITY
}

void GestionReseau::signauxTask(void *p)
{
    Node *node;
    node = (Node *)p;
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    uint16_t oldValue[2] = {0};

    for (;;)
    {
        for (byte i = 0; i < 2; i++)
        {
            if (signalValue[i] > 0 && oldValue[i] != signalValue[i])
            {
                SignauxCmd::affiche(node->signal[i]->affiche(signalValue[i]));
                oldValue[i] = signalValue[i];
                // debug.printf("[GestionReseau %d] Fonction signauxTask\n", __LINE__);
            }
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
    }
}

void IRAM_ATTR GestionReseau::loopTask(void *pvParameters)
{
    Node *node;
    node = (Node *)pvParameters;

    enum : bool
    {
        horaire,
        antiHor
    };

    uint8_t index = 0;
    bool sens0 = 0;
    bool sens1 = 0;
    bool access = 0;
    bool busy = 0;

    uint16_t oldLocAddress = 0;
    uint8_t oldLocSpeed = 0;
    uint8_t oldLocSens = 0;
    uint8_t comptCmdLoco = 0;

#ifdef debug
    char *cantonName;
    cantonName = new char[4];
#endif

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        /*************************************************************************************
         * Occupation du canton
         * et etat des capteurs
         ************************************************************************************/

        if (!node->busy())
        {
            node->sensor[horaire].state(LOW); // Desactivation des capteurs ponctuels si aucune loco reconnue
            node->sensor[antiHor].state(LOW);
            node->loco.speed(0);
            node->loco.sens(0);
        }
        else
            // Adapte la vitesse de la loco à la vitesse maxi du canton
            if (node->maxSpeed() < node->loco.speed())
                node->loco.speed(node->maxSpeed());

        // Sens de roulage des locos,
        if (node->sensor[antiHor].state() && !node->sensor[horaire].state())
            node->loco.sens(2); // Anti Horaire
        else if (node->sensor[horaire].state() && !node->sensor[antiHor].state())
            node->loco.sens(1); // Horaire

        /*************************************************************************************
         * Information aupres des satellites environnants
         ************************************************************************************/

        // En fonction des aiguilles qui appartiennent a ce canton et de leurs positions
        // on en recherche quel est le SP1 et le SM1
        auto rechercheSat = [node](bool satPos) -> byte
        {
            uint8_t idxA = 0; // Pour horaire
            uint8_t idxS = 0;

            if (satPos == 1) // Pour anti-horaire
            {
                idxA = 3;
                idxS = 4;
            }

            uint8_t $idx = idxS;

            if (node->aig[0 + idxA] != nullptr) // L'aiguille 0 à horaire (ou 3 à anti-horaire) existe
            {
                if (node->aig[0 + idxA]->estDroit()) // L'aiguille 0 à horaire (ou 3 à anti-horaire) est droite
                {
                    $idx = 0 + idxS;
                    if (node->aig[1 + idxA] != nullptr) // L'aiguille 1 à horaire (ou 4 à anti-horaire) existe
                    {
                        if (!node->aig[1 + idxA]->estDroit()) // L'aiguille 1 à horaire (ou 4 à anti-horaire) est droite
                            $idx = 0 + idxS;
                        else
                            $idx = 1 + idxS; // L'aiguille 1 à horaire (ou 4 à anti-horaire) est déviée
                    }
                }
                else // L'aiguille 0 à horaire (ou 3 à anti-horaire) est déviée
                {
                    $idx = 1 + idxS;
                    if (node->aig[2 + idxA] != nullptr) // L'aiguille 2 à horaire (ou 5 à anti-horaire) existe
                    {
                        if (node->aig[2 + idxA]->estDroit())
                            $idx = 2 + idxS; // L'aiguille 2 à horaire (ou 5 à anti-horaire) est droite
                        else
                            $idx = 3 + idxS; // L'aiguille 2 à horaire (ou 5 à anti-horaire) est déviée
                    }
                }
            }
            return $idx;
        };
        node->SP1_idx(rechercheSat(horaire));
        node->SM1_idx(rechercheSat(antiHor));

        // debug.printf("[GestionReseau %d] node->SP1_idx = %d\n", __LINE__, node->SP1_idx());
        // debug.printf("[GestionReseau %d] node->SM1_idx = %d\n", __LINE__, node->SM1_idx());

        /*************************************************************************************
         * Envoi sur le bus CAN des informations concernant ce satellite
         ************************************************************************************/

        uint8_t nodeP_SM1_ID = 0;
        bool nodeP_SM2_ACCES = false;
        bool nodeP_SM2_BUSY = false;
        if (node->nodeP[node->SP1_idx()] != nullptr)
        {
            nodeP_SM1_ID = node->nodeP[node->SP1_idx()]->ID();
            nodeP_SM2_ACCES = node->SP2_acces();
            nodeP_SM2_BUSY = node->SP2_busy();
        }

        uint8_t nodeP_SP1_ID = 0;
        bool nodeP_SP2_ACCES = false;
        bool nodeP_SP2_BUSY = false;
        if (node->nodeP[node->SM1_idx()] != nullptr)
        {
            nodeP_SP1_ID = node->nodeP[node->SM1_idx()]->ID();
            nodeP_SP2_ACCES = node->SM2_acces();
            nodeP_SP2_BUSY = node->SM2_busy();
        }

        // debug.printf("[GestionReseau %d ] nodeP_SP1_ID : %d\n", __LINE__, nodeP_SM1_ID);
        // debug.printf("[GestionReseau %d ] nodeP_SM1_ID : %d\n", __LINE__, nodeP_SP1_ID);
        // debug.printf("[GestionReseau %d ] this node busy : %d\n", __LINE__, node->busy());

        CanMsg::sendMsg(1, 0xE0, node->ID(), 0xE0, 0,
                        (node->loco.address() & 0xFF00) >> 8,
                        node->loco.address() & 0x00FF,
                        nodeP_SP1_ID,
                        nodeP_SM1_ID,
                        nodeP_SP2_ACCES,
                        nodeP_SP2_BUSY,
                        nodeP_SM2_ACCES,
                        nodeP_SM2_BUSY);


        enum : uint8_t
        {
            Orange,
            Rouge,
            Vert,
            Carre,
            Ralentissement,
            RRalentissement
        };

        /*************************************************************************************
         *
         ************************************************************************************/

        for (byte i = 0; i < 2; i++)
        {
            switch (i)
            {
            case 0:
                index = node->SP1_idx();
                sens0 = horaire;
                sens1 = antiHor;
                access = node->SP2_acces();
                busy = node->SP2_busy();
#ifdef debug
                strcpy(cantonName, "SP1");
#endif
                break;
            case 1:
                index = node->SM1_idx();
                sens0 = antiHor;
                sens1 = horaire;
                access = node->SM2_acces();
                busy = node->SM2_busy();
#ifdef debug
                strcpy(cantonName, "SM1");
#endif
                break;
            }

            //  debug.printf("[GestionReseau %d] index %d \n", __LINE__, index);
            //  debug.printf("[GestionReseau %d] node->sensor[sens0].state() %d \n", __LINE__, node->sensor[sens0].state());
            //  debug.printf("[GestionReseau %d] node->sensor[sens1].state() %d \n", __LINE__, node->sensor[sens1].state());

            if (node->nodeP[index] != nullptr)
            {
                if (node->nodeP[index]->acces()) // Le canton SP1/SM1 est accessible
                {
                    // debug.printf("[GestionReseau %d] Le canton %s est accessible\n", __LINE__, cantonName);
                    if (node->nodeP[index]->busy()) // Le canton SP1/SM1 est occupé
                    {
                        // debug.printf("[GestionReseau %d] Le canton %s est accessible mais occupe\n", __LINE__, cantonName);
                        signalValue[i] = Rouge;
                        //if (node->loco.sens() == 1)
                        //{
                            if (node->sensor[sens1].state())
                                node->loco.stop();
                            else if (node->sensor[sens0].state())
                                node->loco.speed(30);
                        //}
                        
                        // if (node->loco.sens() == 2)
                        // {
                        //     if (node->sensor[antiHor].state())
                        //         node->loco.stop();
                        //     else if (node->sensor[horaire].state())
                        //         node->loco.ralentis(30);
                        // }
                    }
                    else // Le canton SP1/SM1 est accessible et libre
                    {
                        // debug.printf("[GestionReseau %d] Le canton %s est accessible et libre\n", __LINE__, cantonName);
                        signalValue[i] = Vert;

                        if (access) // Le canton SP2 est-il accessible ?
                        {
                            // debug.printf("[GestionReseau %d] SP2 accessible\n", __LINE__);
                            if (busy) // Le canton SP2 est-il occupé ?
                            {
                                // debug.printf("[GestionReseau %d] SP2 occupe\n", __LINE__);
                                signalValue[i] = Ralentissement;
                            }
                            else // Le canton SP2 n'est pas occupé
                            {
                                // debug.printf("[GestionReseau %d] SP2 libre\n", __LINE__);
                                signalValue[i] = Vert;
                            }
                        }
                        else // Le canton SP2 n'est pas accessible
                        {
                            // debug.printf("[GestionReseau %d] SP2 non accessible\n", __LINE__);
                            signalValue[i] = Ralentissement;
                        }
                    }
                }
                else // Le canton SP1/SM1 est n'est pas accessible
                {
                    // debug.printf("[GestionReseau %d] Le canton %s n'est pas accessible\n", __LINE__, cantonName);
                    signalValue[i] = Carre;

                    // if (node->sensor[sens0].state())
                    //     node->loco.stop();
                    // else
                    //     node->loco.ralentis(30);
                }
            }
            else // Le canton SP1/SM1 n'existe pas
            {
                // debug.printf("[GestionReseau %d] Le canton %s n'existe pas\n", __LINE__, cantonName);
                signalValue[i] = Carre;
                // if (node->sensor[sens0].state())
                //     node->loco.stop();
                // else
                //     node->loco.ralentis(30);
            }
        }

        // Envoi des commandes à la loco
        if (node->loco.address() > 0)
        {
            // if (node->loco.address() != oldLocAddress || node->loco.speed() != oldLocSpeed || node->loco.sens() != oldLocSens)
            if (node->loco.speed() != oldLocSpeed)
                comptCmdLoco = 0;

            if (comptCmdLoco < 5)
            {
                CanMsg::sendMsg(1, 0xF0, node->ID(), 253, 0,
                                (node->loco.address() & 0xFF00) >> 8,
                                node->loco.address() & 0x00FF,
                                node->loco.speed(),
                                node->loco.sens()); // Message à la centrale DCC++

                debug.printf("[GestionReseau %d] Loco %d vitesse %d\n", __LINE__, node->loco.address(), node->loco.speed());

                oldLocAddress = node->loco.address();
                oldLocSpeed = node->loco.speed();
                oldLocSens = node->loco.sens();
                comptCmdLoco++;
            }
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(50));
    }
}