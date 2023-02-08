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
	bool ActualLed1State =false; 
	bool ActualLed2State = false;

	uint32_t i = 0;

	if( iIndex == 0)
	{
		for( i = 0; i < iNumParams; i++)
		{
			if(strcmp(pcParam[i],"led") == 0)
			{
				if(strcmp(pcValue[i],"1") == 0)
				{
					ActualLed1State = true;
				}

				else if(strcmp(pcValue[i],"2") == 0)
				{
					ActualLed2State = true;
				}
			}
		  }
	}


	if(ActualLed1State == true && LedNum1 == false){
		LedNum1 = true;
		GPIO_PinSet(GPIOB,BLUE_LED);
		DEBUGL(DL_INFO,"Uruchamiam Diode Niebieska \n\r");
	}
	if(ActualLed1State == false && LedNum1 == true){
		LedNum1 = false;
		GPIO_PinReset(GPIOB,BLUE_LED);
		DEBUGL(DL_INFO,"Wylaczam Diode Niebieska \n\r");
	}

	if(ActualLed2State == true && LedNum2 == false){
		LedNum2 = true;
		GPIO_PinSet(GPIOB,GREEN_LED);
		DEBUGL(DL_INFO,"Uruchamiam Diode Zielona \n\r");
	}
	if(ActualLed2State == false && LedNum2 == true){
		LedNum2 = false;
		GPIO_PinReset(GPIOB,GREEN_LED);
		DEBUGL(DL_INFO,"Wylaczam Diode Zielona \n\r");
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

