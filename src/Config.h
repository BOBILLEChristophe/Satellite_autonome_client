/*

  Config.h

  Sur les cartes qui utilisent un module ESP32-WROVER pour avoir plus de RAM,
  les pins GPIO16 et GPIO17 ne sont pas disponibles car ils sont utilisés en interne par la PSRAM.  

  ESP32 datasheet : https://www.espressif.com/sites/default/files/documentation/esp32-wrover-e_esp32-wrover-ie_datasheet_en.pdf

  Pin mapping pour cette application : https://alkans.fr/locoduino/satellites_autonomes/sat_autonome_pinMapping_v2.pdf

*/

#ifndef __CONFIG__
#define __CONFIG__

#include <Arduino.h>

enum // Index des satellites périphériques
{
  p00,
  p01,
  p10,
  p11,
  m00,
  m01,
  m10,
  m11
};

/* ----- Options   -------------------*/
//#define SAUV_BY_MAIN  // Sauvegardes automatiques des paramètres commandées par la carte Main toutes les minutes
#define CHIP_INFO
#define RAILCOM
//#define RFID

/* ----- Debug   -------------------*/
//#define DEBUG
#define debug Serial
//#define TEST_MEMORY_TASK

/* ---------------------------------*/
#define NO_ID 255
#define NO_PIN 255

/* ----- CAN ----------------------*/
#define CAN_RX GPIO_NUM_22
#define CAN_TX GPIO_NUM_23
#define CAN_BITRATE 1000UL * 1000UL // 1 Mb/s

/* ----- Node ----------------------*/
const uint8_t nodePsize = 8;
const uint8_t aigSize = 6;
const uint8_t sensorSize = 2;

/* ----- Railcom -------------------*/
#define NB_ADDRESS_TO_COMPARE 100 // Nombre de valeurs à comparer pour obtenir l'adresse de la loco
#ifdef RAILCOM
#define RAILCOM_RX GPIO_NUM_0
#define RAILCOM_TX GPIO_NUM_17
#endif

/* ----- RFID --------------------*/
#ifdef RFID
#define RST_PIN GPIO_NUM_26
#define SCL_PIN GPIO_NUM_14
#define SDA_PIN GPIO_NUM_27
//#define IRQ_PIN GPIO_NUM_25
#define TEMPO_RFID 1UL * 1UL // tempo entre 2 lectures => 1 * 1UL = 1/1000 sec
#endif

/* ----- Sensors ------------------*/
#define CAPT_PONCT_ANTIHOR_PIN GPIO_NUM_33
#define CAPT_PONCT_HORAIRE_PIN GPIO_NUM_15
#define CAPT_PONCT_TEMPO 50UL

/* ----- Registres a decalage ------*/
#define SHREG_PIN_VERROU GPIO_NUM_4
#define SHREG_PIN_HORLOGE GPIO_NUM_5
#define SHREG_PIN_DATA GPIO_NUM_18

/* ----- Découverte ---------------*/
#define INTER_DEV_1 GPIO_NUM_34    // Broche du dip switch pour inter1 dévié
#define INTER_DEV_2 GPIO_NUM_39    // Broche du dip switch pour inter2 dévié
#define BTN_SAT_PLUS GPIO_NUM_36   // Bouton de validation
#define BTN_SAT_MOINS GPIO_NUM_35  // Bouton de validation
#define LED_PIN_DISCOV GPIO_NUM_32 // Led

/* ----- Aiguilles -----------------*/
#define AIG_PIN_SIGNAL_0 GPIO_NUM_19
#define AIG_PIN_SIGNAL_1 GPIO_NUM_21
#define AIG_PIN_SIGNAL_2 GPIO_NUM_2
#define AIG_PIN_SIGNAL_3 GPIO_NUM_13
#define AIG_SPEED 6000.0000

#endif
