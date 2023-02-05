#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__


#define CHECKSUM_BY_HARDWARE 1

#define LWIP_PROVIDE_ERRNO

/*https://lwip-users.nongnu.narkive.com/afAK6Zqe/no-ping-response*/
#ifdef CHECKSUM_BY_HARDWARE
#define CHECKSUM_GEN_IP 0
#define CHECKSUM_GEN_UDP 0
#define CHECKSUM_GEN_TCP 0
#define CHECKSUM_CHECK_IP 0
#define CHECKSUM_CHECK_UDP 0
#define CHECKSUM_CHECK_TCP 0
#define CHECKSUM_GEN_ICMP 0
#else
#define CHECKSUM_GEN_IP 1
#define CHECKSUM_GEN_UDP 1
#define CHECKSUM_GEN_TCP 1
#define CHECKSUM_CHECK_IP 1
#define CHECKSUM_CHECK_UDP 1
#define CHECKSUM_CHECK_TCP 1
#define CHECKSUM_GEN_ICMP 1
#endif


#define 	WITH_RTOS 		0

#define 	NO_SYS			1

#define 	LWIP_NETCONN	0

#define 	LWIP_SOCKET		0

#define 	LWIP_DHCP		1

#define		LWIP_NETIF_LINK_CALLBACK		1

#define 	SYS_LIGHTWEIGHT_PROT			0

#define		LWIP_HTTPD						1

#define		LWIP_HTTPD_SSI					1

// #define HTTPD_USE_CUSTOM_FSDATA 1
#define HTTPD_FSDATA_FILE "./../../../../Config/LWIP/Http_fsdata/fsdata.c"


#endif
