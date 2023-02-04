#include "leds.h"
#include "System.h"
#include "stdio.h"
#include "UART3_dlt.h"
#include "DLTuc.h"
#include "eth_nuc_f767.h"

ETH_InitTypeDef ETH_InitStruct_test;

int main(void)
{
    nucleo_led_init();
    PLL_Config200Mhz();
    ConfigSysTick1ms();
    UART3InitTransmitWithDMAand_ucDLTlib();

    ETH_gpio_rcc_init();
    ETH_StructInit(&ETH_InitStruct_test);
    ETH_Init(&ETH_InitStruct_test, LAN8742A_PHY_ADDRESS);

    static uint32_t EthLinkTimer = 0;
    static ETH_CallStatus_Type LinkState = 0;
    static ETH_CallStatus_Type PrevLinkState = 0;
    LinkState = GetLinkState(LAN8742A_PHY_ADDRESS);
    PrevLinkState= LinkState;
    DEBUGL(DL_INFO, "Ethernet Cable  %s", LinkState == OK?  "Connected":"Disconnected")

    while(1)
    {
        if(GetSysTime() - EthLinkTimer > 200)
        {
            tooglePIN(GPIOB,GREEN_LED);

            LinkState = GetLinkState(LAN8742A_PHY_ADDRESS);

            if(LinkState != PrevLinkState)
            {
                DEBUGL(DL_INFO, "Ethernet Cable  %s", LinkState == OK?  "Connected":"Disconnected")
                PrevLinkState = LinkState;
            }
        }
    }
}

