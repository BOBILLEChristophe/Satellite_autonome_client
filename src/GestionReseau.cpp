/*

  GestionReseau.cpp


*/

#include "GestionReseau.h"

uint16_t GestionReseau::signalValue[2] = {0};

void GestionReseau::setup(Node *node)
{
    xTaskCreatePinnedToCore(signauxTask, "SignauxTask", 1 * 1024, (void *)node, 3, NULL, 1); // Création de la tâches pour le traitement
    xTaskCreatePinnedToCore(loopTask, "LoopTask", 4 * 1024, (void *)node, 5, NULL, 1);
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
        // debug.printf("[GestionReseau %d] Fonction signauxTask\n", __LINE__);

        for (byte i = 0; i < 2; i++)
        {
            if (signalValue[i] > 0 && oldValue[i] != signalValue[i])
            {
                SignauxCmd::affiche(node->signal[i]->affiche(signalValue[i]));
                oldValue[i] = signalValue[i];
            }
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(2000));
    }
}

void GestionReseau::loopTask(void *pvParameters)
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
    String cantonName;

    uint16_t valueToSend;

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {

        /*************************************************************************************
         * Occupation du canton detectee par Railcom
         * et etat des capteurs
         ************************************************************************************/

        if (!node->busy())
        {
            node->sensor[horaire].state(LOW); // Desactivation des capteurs ponctuels si aucune loco reconnue
            node->sensor[antiHor].state(LOW);
            node->loco.m_comptCmd0 = 0;
            node->loco.m_comptCmd30 = 0;
            node->loco.sens(0);
        }

        // Sens de roulage des locos,
        if (node->sensor[antiHor].state() && !node->sensor[horaire].state())
            node->loco.sens(1);
        else if (node->sensor[horaire].state() && !node->sensor[antiHor].state())
            node->loco.sens(2);

        /*************************************************************************************
         * Information aupres des satellites environnants
         ************************************************************************************/

        // En fonction des aiguilles qui appartiennent a ce canton et de leur position
        // on en recherche quel est le SP1 et le SM1
        auto rechercheSat = [node](bool satPos) -> byte
        {
            uint8_t idxA = 0;
            uint8_t idxS = 0;

            if (satPos == 1)
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

        CanMsg::sendMsg(0, node->ID(), 0xE1, 0xE1,
                        node->busy(),
                        nodeP_SP1_ID,
                        nodeP_SM1_ID,
                        nodeP_SP2_ACCES,
                        nodeP_SP2_BUSY,
                        nodeP_SM2_ACCES,
                        nodeP_SM2_BUSY);

        // Il sera possible de completer les infos envoyees pour plus d'interactivite
        // comme l'adresse de la loco, le sens de roulage...

        // CanMsg::sendMsg(0, node->ID(), 0xE2, 0xE2,
        //                 node->busy(),
        //                 nodeP_SP1_ID,
        //                 nodeP_SM1_ID,
        //                 node->loco.address() & 0xFF00,
        //                 node->loco.address() & 0x00FF);

        /*************************************************************************************
         * Commande des locomotives et de la signalisation
         ************************************************************************************/

        auto cmdLoco = [node](const uint8_t speed, const uint8_t direction)
        {
            // debug.printf("[GestionReseau %d] Test passage programme %d\n ", __LINE__, speed);
            while (node->loco.address() == 0)
                ;

            uint8_t *ptCompt(nullptr);
            // debug.printf("[GestionReseau %d] commande loco %d\n ", __LINE__, speed);
            switch (speed) // Selection du compteur selon la vitesse voulue
            {
            case 0:
                ptCompt = &node->loco.m_comptCmd0;
                break;
            case 30:
                ptCompt = &node->loco.m_comptCmd30;
                break;
            }

            if (*ptCompt < 5)
            {
                CanMsg::sendMsg(1, node->ID(), 253, 0xF0, node->loco.address() & 0xFF00, node->loco.address() & 0x00FF, speed, direction); // Message à la centrale DCC++
                ++*ptCompt;
                vTaskDelay(pdMS_TO_TICKS(200));
                // debug.printf("[GestionReseau %d] Envoi de commade loco vitesse %d\n ", __LINE__, speed);
            }
        };

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
                cantonName = "SP1";
                break;
            case 1:
                index = node->SM1_idx();
                sens0 = horaire;
                sens1 = antiHor;
                access = node->SM2_acces();
                busy = node->SM2_busy();
                cantonName = "SM1";
                break;
            }

            // debug.printf("[GestionReseau %d] index %d \n", __LINE__, index);

            // debug.printf("[GestionReseau %d] node->sensor[sens0].state() %d \n", __LINE__, node->sensor[sens0].state());
            // debug.printf("[GestionReseau %d] node->sensor[sens1].state() %d \n", __LINE__, node->sensor[sens1].state());

            if (node->nodeP[index] != nullptr)
            {
                if (node->nodeP[index]->acces()) // Le canton SP1 est accessible
                {
                    // debug.printf("[GestionReseau %d] Le canton SP1 est accessible\n", __LINE__);
                    if (node->nodeP[index]->busy()) // Le canton SP1 est occupé
                    {

                        //debug.printf("[GestionReseau %d] Le canton %s est accessible mais occupe\n", __LINE__, cantonName);
                        signalValue[i] = Rouge;

                        // Ordre loco Ralentissement à 30
                        if (node->sensor[sens1].state())
                            cmdLoco(30, 1);
                        // arret au franchissement du capteur
                        if (node->sensor[sens0].state())
                            cmdLoco(0, 1);
                    }
                    else // Le canton SP1 est accessible et libre
                    {
                        //debug.printf("[GestionReseau %d] Le canton %s est accessible et libre\n", __LINE__, cantonName);
                        //    Vérification de l'état de SP2
                        //  CanMsg::sendMsg(0, node->ID(), node->nodeP[node->SP1_idx()]->ID(), 0xA5);
                        //  debug.printf("[GestionReseau %d] SP1 : %d\n", __LINE__, node->nodeP[node->SP1_idx()]->ID());
                        //  vTaskDelay(pdMS_TO_TICKS(20)); // Attente de la reponse

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
                else // Le canton SP1 est n'est pas accessible
                {
                    //debug.printf("[GestionReseau %d] Le canton %s n'est pas accessible\n", __LINE__, cantonName);
                    //    /*
                    //    Signalisation ???
                    //    */

                    // Ordre loco Ralentissement à 30 if (node->sensor[sens1].state())
                    cmdLoco(30, 1);
                    // arret au franchissement du capteur
                    if (node->sensor[sens0].state())
                        cmdLoco(0, 1);
                }
            }
            else // Le canton SP1 n'existe pas
            {
                //debug.printf("[GestionReseau %d] Le canton %s n'existe pas\n", __LINE__, cantonName);
                signalValue[i] = Carre;

                // Ordre loco Ralentissement à 30
                if (node->sensor[sens1].state())
                    cmdLoco(30, 1);
                // arret au franchissement du capteur
                if (node->sensor[sens0].state())
                    cmdLoco(0, 1);
            }
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(500));
    }
}