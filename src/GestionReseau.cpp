/*

  GestionReseau.cpp


*/

#include "GestionReseau.h"

bool GestionReseau::isRun{false};
void GestionReseau::run(bool run) { GestionReseau::isRun = run; }

void GestionReseau::setup(Node *node)
{
    debug.println("GestionResau::setup");
    TaskHandle_t loopHandle = NULL;
    xTaskCreate(loop, "Loop", 4 * 1024, (void *)node, 10, &loopHandle); // Création de la tâches pour le traitement
}

void GestionReseau::loop(void *pvParameters)
{
    Node *node;
    node = (Node *)pvParameters;

    enum
    {
        horaire,
        antiHor
    };

    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        if (!Settings::discoveryOn())
        {
            /*************************************************************************************
             * Occupation du canton detectee par Railcom
             * et etat des capteurs
             ************************************************************************************/

            // Desactivation des capteurs ponctuels si canton vide
            if (!node->busy())
            {
                node->sensor[horaire].state(LOW);
                node->sensor[antiHor].state(LOW);
            }
            else
            {
                // Pour déterminer le sens de roulage des locos,
                if (node->sensor[antiHor].state() && !node->sensor[horaire].state())
                {
                    node->loco.sens(horaire);
                    debug.printf("Sens de roulage horaire");
                }
                else if (node->sensor[horaire].state() && !node->sensor[antiHor].state())
                {
                    node->loco.sens(antiHor);
                    debug.printf("Sens de roulage anti-horaire");
                }
            }

            /*************************************************************************************
             * Information aupres des satellites environnants
             ************************************************************************************/

            // En fonction des aiguilles qui appartiennent a ce canton et de leur position
            // on en deduit quel est le SP1 et le SM1
            auto rechercheSat = [node](bool satPos) -> byte
            {
                uint8_t idxA = 0;
                uint8_t idxS = 0;

                if (satPos == 1)
                {
                    idxA = 3;
                    idxS = 4;
                }

                uint8_t $idx = NO_ID;

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
            node->SP1_idx(rechercheSat(0));
            node->SM1_idx(rechercheSat(1));

#ifdef DEBUG
            debug.printf("node->SP1_idx() %d\n", node->SP1_idx());
            debug.printf("node->SM1_idx() %d\n", node->SM1_idx());
#endif

            // Ce satellite envoie maintenant une demande d'informations à son SP1 et son SM1
            const uint8_t idx[2] = {node->SP1_idx(), node->SM1_idx()};
            for (byte i = 0; i < 2; i++)
            {
                if (idx[i] < NO_ID)
                {
                    if (node->nodeP[idx[i]] != nullptr)
                    {
                        CanMsg::sendMsg(0, node->ID(), node->nodeP[idx[i]]->ID(), 0xA1 + i);
                        vTaskDelay(20 / portTICK_PERIOD_MS); // Attente de la reponse
                    }
                }
            }

            if (node->nodeP[node->SP1_idx()] != nullptr)
            {
                CanMsg::sendMsg(0, node->ID(), node->nodeP[node->SP1_idx()]->ID(), 0xA1);
                vTaskDelay(50 / portTICK_PERIOD_MS);
                debug.printf("SP1 occupe : %d\n", node->nodeP[node->SP1_idx()]->busy());
                debug.printf("SP1 accessible : %d\n", node->nodeP[node->SP1_idx()]->acces());
            }
            if (node->nodeP[node->SM1_idx()] != nullptr)
            {
                CanMsg::sendMsg(0, node->ID(), node->nodeP[node->SM1_idx()]->ID(), 0xA2);
                vTaskDelay(50 / portTICK_PERIOD_MS);
                debug.printf("SM1 occupe :  %d\n", node->nodeP[node->SM1_idx()]->busy());
                debug.printf("SM1 accessible :  %d\n", node->nodeP[node->SM1_idx()]->acces());
            }

            /*************************************************************************************
             * Commande des locomotives et de la signalisation
             ************************************************************************************/

            auto cmdLoco = [node](const uint16_t address, const uint8_t speed, const uint8_t direction)
            {
                const uint8_t $address0 = address & 0x00FF;
                const uint8_t $address1 = address & 0xFF00;
                CanMsg::sendMsg(1, node->ID(), 253, 0xF0, $address1, $address0, speed, direction); // Message à la centrale DCC++
            };

            enum
            {
                rouge,
                vert,
                orange,
                carre,
                ralentissement,
                Rralentissement
            };

            if (node->nodeP[node->SP1_idx()] != nullptr)
            {
                if (node->nodeP[node->SP1_idx()]->acces()) // Le canton SP1 est accessible
                {
                    debug.printf("Le canton SP1 est accessible\n");
                    if (node->nodeP[node->SP1_idx()]->busy()) // Le canton SP1 est occupé
                    {
                        debug.printf("Le canton SP1 est occupe\n");
                        node->signal[0]->affiche(rouge); // Signalisation Rouge + oeilleton
                        //  Ordre loco Ralentissement à 30
                        if (node->sensor[1].state() && ! node->sensor[0].state())
                            cmdLoco(node->loco.address(), 30, 1);
                        // arret au franchissement du capteur
                        if (node->sensor[0].state())
                            cmdLoco(node->loco.address(), 0, 1);
                    }
                    else // Le canton SP1 est accessible et libre
                    {    // Vérification de l'état de SP2
                        // debug.printf("SP1 = node->nodeP[%d] : libre\n", node->SP1_idx());
                        debug.printf("Le canton SP1 est accessible et libre\n");
                        CanMsg::sendMsg(0, node->ID(), 0, 0xA3, node->nodeP[node->SP1_idx()]->ID());
                        vTaskDelay(20 / portTICK_PERIOD_MS);

                        if (node->SP2_acces()) // Le canton SP2 est-il accessible ?
                        {
                            debug.printf("SP2 accessible\n");
                            if (node->SP2_busy()) // Le canton SP2 est-il occupé ?
                            {
                                debug.printf("SP2 occupe\n");
                                //  Signalisation Ralentissement
                            }
                            else // Le canton SP2 n'est pas occupé
                            {
                                debug.printf("SP2 libre\n");
                                // Signalisation vert
                            }
                        }
                        else // Le canton SP2 n'est pas accessible
                        {
                            debug.printf("SP2 non accessible\n");
                            // Signalisation Ralentissement
                        }
                    }
                }
                else // Le canton SP1 est n'est pas accessible
                {
                    debug.printf("Le canton SP1 est n'est pas accessible\n");
                    /*
                    Signalisation ???
                    */
                    // Ordre loco Ralentissement à 30
                    if (node->sensor[1].state() && !node->sensor[0].state())
                        cmdLoco(node->loco.address(), 30, 1);
                    // arret au franchissement du capteur
                    if (node->sensor[0].state())
                        cmdLoco(node->loco.address(), 0, 1);
                }
            }
            else // Le canton SP1 n'existe pas
            {
                debug.printf("Le canton SP1 n'existe pas\n");
                /*
                Signalisation Carré
                */

                //  Ordre loco Ralentissement à 30
                if (node->sensor[1].state() && !node->sensor[0].state())
                    cmdLoco(node->loco.address(), 30, 1);
                // arret au franchissement du capteur
                if (node->sensor[0].state())
                    cmdLoco(node->loco.address(), 0, 1);
            }
        }
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(200));
    }
}