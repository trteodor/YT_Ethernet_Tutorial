#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__


#define CHECKSUM_BY_HARDWARE 

#define LWIP_PROVIDE_ERRNO

// #define LWIP_ERRNO_INCLUDE

// #define LWIP_ERRNO_STDINCLUDE



#define 	WITH_RTOS 		0

#define 	NO_SYS			1

#define 	LWIP_NETCONN	0

#define 	LWIP_SOCKET		0

#define 	LWIP_DHCP		1

#define		LWIP_NETIF_LINK_CALLBACK		1

#define 	SYS_LIGHTWEIGHT_PROT			0

#define		LWIP_HTTPD						1

#define		LWIP_HTTPD_SSI					1

#endif
