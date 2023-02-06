#ifndef __CGI_PROCESS_H__
#define __CGI_PROCESS_H__

#include <stdint.h>
#include "leds.h"


#define NUM_OF_CGIS		1

#define NUM_OF_TAGS 	3

uint16_t  ssi_handler(int iIndex,char *pcInsert, int iInsertLen);

#endif
