#include "lwip.h"



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
	 }
	 else
	 {
		 netif_set_down(&gnetif);
	 }

	/*Set the link change callback function*/
	//  netif_set_link_callback(&gnetif,ethernetif_update_config);

	/*Start dhcp*/
	 dhcp_start(&gnetif);

}

/*This function will read received packets from the ethernet buffers
 * and send them to lwip stack*/

void lwip_process_data(void)
{
	ethernetif_input(&gnetif);
	// ethernetif_handle_link_state(&gnetif);
	sys_check_timeouts();
}
