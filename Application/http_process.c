#include <string.h>
#include <stdio.h>
#include "http_process.h"
#include <stdbool.h>
#include "System.h"

#include "lwip/apps/httpd.h"
#include "DLTuc.h"
#include "leds.h"

bool LedNum1, LedNum2;

char const * ssi_tags[]= {"tag1","tag2","tag3"};
char const ** tags = ssi_tags;


const char * led_cgi_handler (int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

const tCGI LED_CGI ={"/leds.cgi",led_cgi_handler};

tCGI CGI_ARR[NUM_OF_CGIS];

const char * led_cgi_handler (int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{

	uint32_t i = 0;

	if( iIndex == 0)
	{
		LedNum1 = LedNum2 = 0;
		DEBUGL(DL_INFO,"Wylacz wszystkie diody \n\r");
		GPIO_PinReset(GPIOB,GREEN_LED);
		GPIO_PinReset(GPIOB,BLUE_LED);

		for( i = 0; i < iNumParams; i++)
		{
			if(strcmp(pcParam[i],"led") == 0)
			{
				if(strcmp(pcValue[i],"1") == 0)
				{
					DEBUGL(DL_INFO,"LedNum1 Green Zapal \n\r");
					GPIO_PinSet(GPIOB,GREEN_LED);
					LedNum1 = 1;
				}

				else if(strcmp(pcValue[i],"2") == 0)
				{
					DEBUGL(DL_INFO,"LedNum2 BLUE Zapal \n\r");
					GPIO_PinSet(GPIOB,BLUE_LED);
					LedNum2 = 1;
				}
			}
		  }
	}
	return "/index.shtml";
}


uint16_t  ssi_handler(int iIndex,char *pcInsert, int iInsertLen)
{
	switch(iIndex){
		case 0:
			sprintf(pcInsert, "%d",(int)GetSysTime());
			return strlen(pcInsert);
			break;

		case 1:
			if(LedNum1 == true)
			{
				sprintf(pcInsert,"<input value=\"1\" name=\"led\" class=\"largerCheckbox\" type=\"checkbox\" checked>");
				return strlen(pcInsert);
			}
			else if(LedNum1 == false)
			{
				sprintf(pcInsert,"<input value=\"1\" name=\"led\" class=\"largerCheckbox\" type=\"checkbox\">");
				return strlen(pcInsert);
			}
		case 2:
			if(LedNum2 == true)
			{
				sprintf(pcInsert,"<input value=\"2\" name=\"led\" class=\"largerCheckbox\" type=\"checkbox\" checked>");
				return strlen(pcInsert);
			}
			else if(LedNum2 == false)
			{
				sprintf(pcInsert,"<input value=\"2\" name=\"led\" class=\"largerCheckbox\" type=\"checkbox\">");
				return strlen(pcInsert);
			}
			break;

		default:
			break;

	}
	return 0;
}

