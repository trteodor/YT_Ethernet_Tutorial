#ifndef __lwip_h__
#define __lwip_h__

#include "opt.h"
#include  "lwip/mem.h"
#include "lwip/memp.h"
#include "netif/etharp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "ethernetif.h"
#include "eth_nuc_f767.h"


void lwip_lib_init(void);
void lwip_process_data(void);


#endif
