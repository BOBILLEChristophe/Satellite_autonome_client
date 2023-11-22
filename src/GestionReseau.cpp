/*

  GestionReseau.cpp


*/

#include "GestionReseau.h"

bool GestionReseau::isRun{false};
void GestionReseau::run(bool run) { GestionReseau::isRun = run; }

void GestionReseau::setup(Node *node)
{
    TaskHandle_t loopHandle = NULL;
    xTaskCreate(loop, "Loop", 4 * 1024, (void *)node, 10, &loopHandle); // Création de la tâches pour le traitement
}

void GestionReseau::loop(void *pvParameters)
{
    Node *node;
    node = (Node *)pvParameters;

    enum : bool
    {
        horaire,
        antiHor
    };

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        if (!Settings::discoveryOn()) // La gestion de reseau ne fonctionne que si le mode discovery
                                      // est desactive
        {
            /*************************************************************************************
             * Occupation du canton detectee par Railcom
             * et etat des capteurs
             ************************************************************************************/

            // Desactivation des capteurs ponctuels si aucune loco reconnue
            if (!node->busy())
            {
                node->sensor[horaire].state(LOW);
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

            uint8_t nodeP_SP1_ID = 0;
            if (node->nodeP[node->SP1_idx()] != nullptr)
                nodeP_SP1_ID = node->nodeP[node->SP1_idx()]->ID();
            uint8_t nodeP_SM1_ID = 0;
            if (node->nodeP[node->SM1_idx()] != nullptr)
                nodeP_SP1_ID = node->nodeP[node->SM1_idx()]->ID();

            CanMsg::sendMsg(0, node->ID(), 0xE1, 0xE1,
                            node->busy(),
                            nodeP_SM1_ID,
                            nodeP_SP1_ID);

            // Il sera possible de completer les infos envoyees pour plus d'interactivite
            // comme l'adresse de la loco, le sens de roulage...

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
                rouge,
                vert,
                orange,
                carre,
                ralentissement,
                Rralentissement
            };

            /*************************************************************************************
             * Sens de roulage horaire
             ************************************************************************************/

            if (node->nodeP[node->SP1_idx()] != nullptr)
            {
                if (node->nodeP[node->SP1_idx()]->acces()) // Le canton SP1 est accessible
                {
                    // debug.printf("[GestionReseau %d] Le canton SP1 est accessible\n", __LINE__);
                    if (node->nodeP[node->SP1_idx()]->busy()) // Le canton SP1 est occupé
                    {
                        debug.printf("[GestionReseau %d] Le canton SP1 est accessible mais occupe\n", __LINE__);
                        //  node->signal[0]->affiche(rouge); // Signalisation Rouge + oeilleton

                        //  Ordre loco Ralentissement à 30
                        if (node->sensor[antiHor].state())
                            cmdLoco(30, 1);
                        // arret au franchissement du capteur
                        if (node->sensor[horaire].state())
                            cmdLoco(0, 1);
                    }
                    else // Le canton SP1 est accessible et libre
                    {
                        debug.printf("[GestionReseau %d] Le canton SP1 est accessible et libre\n", __LINE__);
                        //   Vérification de l'état de SP2
                        // CanMsg::sendMsg(0, node->ID(), node->nodeP[node->SP1_idx()]->ID(), 0xA5);
                        // debug.printf("[GestionReseau %d] SP1 : %d\n", __LINE__, node->nodeP[node->SP1_idx()]->ID());
                        // vTaskDelay(pdMS_TO_TICKS(20)); // Attente de la reponse

                        if (node->SP2_acces()) // Le canton SP2 est-il accessible ?
                        {
                            // debug.printf("[GestionReseau %d] SP2 accessible\n", __LINE__);
                            if (node->SP2_busy()) // Le canton SP2 est-il occupé ?
                            {
                                // debug.printf("[GestionReseau %d] SP2 occupe\n", __LINE__);
                                //    Signalisation Ralentissement
                            }
                            else // Le canton SP2 n'est pas occupé
                            {
                                // debug.printf("[GestionReseau %d] SP2 libre\n", __LINE__);
                                //   Signalisation vert
                            }
                        }
                        else // Le canton SP2 n'est pas accessible
                        {
                            // debug.printf("[GestionReseau %d] SP2 non accessible\n", __LINE__);
                            //   Signalisation Ralentissement
                        }
                    }
                }
                else // Le canton SP1 est n'est pas accessible
                {
                    debug.printf("[GestionReseau %d] Le canton SP1 est n'est pas accessible\n", __LINE__);
                    //    /*
                    //    Signalisation ???
                    //    */

                    //  Ordre loco Ralentissement à 30
                    if (node->sensor[antiHor].state())
                        cmdLoco(30, 1);
                    // arret au franchissement du capteur
                    if (node->sensor[horaire].state())
                        cmdLoco(0, 1);
                }
            }
            else // Le canton SP1 n'existe pas
            {
                debug.printf("[GestionReseau %d] Le canton SP1 n'existe pas\n", __LINE__);
                /*
                Signalisation Carré
                */

                //  Ordre loco Ralentissement à 30
                if (node->sensor[antiHor].state())
                    cmdLoco(30, 1);
                // arret au franchissement du capteur
                if (node->sensor[horaire].state())
                    cmdLoco(0, 1);
            }

            
            /*************************************************************************************
             * Sens de roulage anti-horaire
             ************************************************************************************/

            if (node->nodeP[node->SM1_idx()] != nullptr)
            {
                if (node->nodeP[node->SM1_idx()]->acces()) // Le canton SP1 est accessible
                {
                    // debug.printf("[GestionReseau %d] Le canton SP1 est accessible\n", __LINE__);
                    if (node->nodeP[node->SM1_idx()]->busy()) // Le canton SP1 est occupé
                    {
                        debug.printf("[GestionReseau %d] Le canton SM1 est accessible mais occupe\n", __LINE__);
                        //  node->signal[3]->affiche(rouge); // Signalisation Rouge + oeilleton

                        //  Ordre loco Ralentissement à 30
                        if (node->sensor[horaire].state())
                            cmdLoco(30, 1);
                        // arret au franchissement du capteur
                        if (node->sensor[antiHor].state())
                            cmdLoco(0, 1);
                    }
                    else // Le canton SP1 est accessible et libre
                    {
                        debug.printf("[GestionReseau %d] Le canton SM1 est accessible et libre\n", __LINE__);
                
                        if (node->SM2_acces()) // Le canton SP2 est-il accessible ?
                        {
                            // debug.printf("[GestionReseau %d] SP2 accessible\n", __LINE__);
                            if (node->SM2_busy()) // Le canton SP2 est-il occupé ?
                            {
                                debug.printf("[GestionReseau %d] SM2 occupe\n", __LINE__);
                                //    Signalisation Ralentissement
                            }
                            else // Le canton SP2 n'est pas occupé
                            {
                                debug.printf("[GestionReseau %d] SM2 libre\n", __LINE__);
                                //   Signalisation vert
                            }
                        }
                        else // Le canton SM2 n'est pas accessible
                        {
                            debug.printf("[GestionReseau %d] SM2 non accessible\n", __LINE__);
                            //   Signalisation Ralentissement
                        }
                    }
                }
                else // Le canton SM1 est n'est pas accessible
                {
                    debug.printf("[GestionReseau %d] Le canton SM1 est n'est pas accessible\n", __LINE__);
                    //    /*
                    //    Signalisation ???
                    //    */

                    //  Ordre loco Ralentissement à 30
                    if (node->sensor[horaire].state())
                        cmdLoco(30, 1);
                    // arret au franchissement du capteur
                    if (node->sensor[antiHor].state())
                        cmdLoco(0, 1);
                }
            }
            else // Le canton SM1 n'existe pas
            {
                debug.printf("[GestionReseau %d] Le canton SM1 n'existe pas\n", __LINE__);
                /*
                Signalisation Carré
                */

                //  Ordre loco Ralentissement à 30
                if (node->sensor[horaire].state())
                    cmdLoco(30, 1);
                // arret au franchissement du capteur
                if (node->sensor[antiHor].state())
                    cmdLoco(0, 1);
            }
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(100));
    }
}