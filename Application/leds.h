

#ifndef _nucleo_leds__
#define _nucleo_leds__

#include "GPIO_f7.h"

#define  GREEN_LED		(1U<<7)  /*Blue Led*/
#define  BLUE_LED		(1U<<14) /*Red Led*/

void nucleo_led_init(void);



#endif //_nucleo_leds__