#include "leds.h"
#include "System.h"
#include "stdio.h"
#include "UART3_dlt.h"
#include "DLTuc.h"
#include "eth_nuc_f767.h"

#include "lwip.h"
#include "httpd.h"
#include "http_process.h"

extern char const ** tags;
extern const tCGI LED_CGI;
extern tCGI CGI_ARR[NUM_OF_CGIS];


extern struct netif gnetif;

int main(void)
{
    nucleo_led_init();
    PLL_Config200Mhz();
    ConfigSysTick1ms();
    UART3InitTransmitWithDMAand_ucDLTlib();

    lwip_lib_init();
	httpd_init();
	http_set_ssi_handler(ssi_handler,(const char **)tags, NUM_OF_TAGS);
	CGI_ARR[0] = LED_CGI;
	http_set_cgi_handlers(CGI_ARR,NUM_OF_CGIS);

    while(1)
    {
        lwip_process_data();
    }
}

