#include "leds.h"
#include "System.h"
#include "stdio.h"
#include "UART3_dlt.h"
#include "DLTuc.h"

int main(void)
{
    nucleo_led_init();
    PLL_Config200Mhz();
    ConfigSysTick1ms();
    UART3InitTransmitWithDMAand_ucDLTlib();


    while(1)
    {
        DelayMs(1000);
        tooglePIN(GPIOB,GREEN_LED);
        DEBUGL(DL_INFO,"Hello Ethernet World!");
        DEBUGL(DL_INFO,"Another Diffrent Log!");
        DEBUGL(DL_INFO,"This is crazy!!");
    }
}

