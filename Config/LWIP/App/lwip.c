#include "lwip.h"
#include "ethernetif.h"
#include "DLTuc.h"

static uint32_t PrevIpAddress = 0;

struct netif gnetif;

ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;

void lwip_lib_init(void)
{
	/*Initialize lwip stack*/
	lwip_init();

	/*Initialize IP address -DHCP mode*/
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;


	/*Add the network interface*/
	 netif_add(&gnetif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&ethernet_input);

	/*Resgister the network interface*/
	 netif_set_default(&gnetif);

	/*Check if link is up*/
	 if(netif_is_link_up(&gnetif))
	 {
		 netif_set_up(&gnetif);
		 dhcp_start(&gnetif);
	 }
	 else
	 {
		 netif_set_down(&gnetif);
	 }

	DEBUGL(DL_INFO,"Initialized Address IP: %s\r\n",ip4addr_ntoa(&gnetif.ip_addr));
	PrevIpAddress= gnetif.ip_addr.addr;
	/*Set the link change callback function*/
	 netif_set_link_callback(&gnetif,ethernetif_update_config);
}

/*This function will read received packets from the ethernet buffers
 * and send them to lwip stack*/

void lwip_process_data(void)
{
	if(PrevIpAddress != gnetif.ip_addr.addr)
	{
		PrevIpAddress = gnetif.ip_addr.addr;
		DEBUGL(DL_INFO,"New Ip Addres Assigned: %s\r\n",ip4addr_ntoa(&gnetif.ip_addr));
	}
	ethernetif_input(&gnetif);
	ethernetif_handle_link_state(&gnetif);
	sys_check_timeouts();
}
