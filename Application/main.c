#include "leds.h"
#include "System.h"
#include "stdio.h"
#include "UART3_dlt.h"
#include "DLTuc.h"
#include "eth_nuc_f767.h"

#include "lwip.h"
#include "lwip/apps/httpd.h"

extern struct netif gnetif;

int main(void)
{
    nucleo_led_init();
    PLL_Config200Mhz();
    ConfigSysTick1ms();
    UART3InitTransmitWithDMAand_ucDLTlib();

    lwip_lib_init();
	httpd_init();

    static uint32_t EthLinkTimer = 0;
    static ETH_CallStatus_Type LinkState = 0;
    static ETH_CallStatus_Type PrevLinkState = 0;
    LinkState = GetLinkState(LAN8742A_PHY_ADDRESS);
    PrevLinkState= LinkState;
    DEBUGL(DL_INFO, "Ethernet Cable  %s", LinkState == OK?  "Connected":"Disconnected")

    while(1)
    {
        lwip_process_data();

        if(GetSysTime() - EthLinkTimer > 200)
        {
            EthLinkTimer = GetSysTime();
            tooglePIN(GPIOB,GREEN_LED);

            DEBUGL(DL_INFO,"IP Address %s\r\n",ip4addr_ntoa(&gnetif.ip_addr));

            LinkState = GetLinkState(LAN8742A_PHY_ADDRESS);

            if(LinkState != PrevLinkState)
            {
                DEBUGL(DL_INFO, "Ethernet Cable  %s", LinkState == OK? "Connected":"Disconnected")
                PrevLinkState = LinkState;
            }
        }
    }
}

