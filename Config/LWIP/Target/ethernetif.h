#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);
err_t ethernetif_input(struct netif *netif);

void ethernetif_update_config(struct netif *netif);
uint8_t ethernetif_handle_link_state(struct netif *netif);

#endif
