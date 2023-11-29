/*


  CanConfig.h

https://dccwiki.com/CAN_Bus
https://fr.wikipedia.org/wiki/Bus_de_donn%C3%A9es_CAN

https://www-kvaser-com.translate.goog/can-protocol-tutorial/?_x_tr_sl=en&_x_tr_tl=fr&_x_tr_hl=fr&_x_tr_pto=rq#:~:text=The%20message%20arbitration%20(the%20process,has%20detected%20an%20idle%20bus.

                  1	2	3	4	5	6	7	8	9	10 11	12	13	14	15	16	17	18	19	20	21	22	23	24	25	26	27	28	29

Priorité	      x	x
ID expediteur			x	x	x	x	x	x	x	x
ID destinataire											x	x	x	x	x	x	x	x
Fonction											        							x	x	x	x	x	x	x	x
Libre		  				                																			0	0	0
                  0	0	0	0	0	0	0	0	0	0									0	0	0	0	0	0	0	0	0	0	0
Masque								                	0	0	0	0	0	0	0	0
                  1	1	1	1	1	1	1	1	1	1	0	0	0	0	0	0	0	0	1	1	1	1	1	1	1	1	1	1	1 = 0x1FF807FF

*/

#ifndef __CAN_CONFIG__
#define __CAN_CONFIG__

#include <Arduino.h>
#include <ACAN_ESP32.h>
#include "Config.h"

class CanConfig
{
private:
public:
  CanConfig() = delete;
  static void setup(const byte);
  //static void setup(const byte, const bool);
};

#endif