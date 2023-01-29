#include "leds.h"
#include "System.h"
#include "stdio.h"
#include "usart3.h"

int main(void)
{
    nucleo_led_init();
    PLL_Config200Mhz();
    ConfigSysTick1ms();
    uart3_tx_init();

    while(1)
    {
        DelayMs(1000);
        tooglePIN(GPIOB,GREEN_LED);
        printf("Hello crazy world! :) \n\r");
    }
}