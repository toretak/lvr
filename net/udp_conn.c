// STELARISWARE
#include "lwip/inet.h"
#include "lwip/udp.h"
#include "lwip/netif.h"
#include <stdint.h>
#include <string.h>

// PROJECT INCLUDES
#include "net/udp_conn.h"
#include "device.h"
#include "ethernetwrapper.h"

//------------------------------------------------------------
struct udp_pcb *g_upcb;
tDeviceSettings *ptrDeviceSettings;
 
/* UDP receive .............................................................*/
static void udp_conn_rx(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
    LWIP_UNUSED_ARG(arg);
    if(p == NULL)
	    return;
    
    udp_sendto(g_upcb, p, addr, port);
    pbuf_free(p);
    DebugMsg("Recieved UDP message: %s\n",p->payload);
}
/* UDP transmit ............................................................*/
void udpConnTx(u8_t *data, u16_t len) {
    if (ptrDeviceSettings->report != 0) {
	DebugMsg("Sending UDP message %s to %d.%d.%d.%d:%d\n",&data,ptrDeviceSettings->reipaddr[0],ptrDeviceSettings->reipaddr[1],ptrDeviceSettings->reipaddr[2],ptrDeviceSettings->reipaddr[3],ptrDeviceSettings->report);
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
        memcpy(p->payload, data, len);
	if((udp_sendto(g_upcb, p,(struct ip_addr*)&ptrDeviceSettings->reipaddr,ptrDeviceSettings->report))!=ERR_OK)
	{
	  DebugMsg("Error UDP packet\n");
	  ledgreen_pinset(1);
	}
	pbuf_free(p);
    }
}
/* UDP initialization ......................................................*/
void udpConnInit(tDeviceSettings *ptrs) {
    ptrDeviceSettings = ptrs;
    g_upcb = udp_new();
    udp_bind(g_upcb, (struct ip_addr*)&ptrs->reipaddr, ptrs->report);
    udp_recv(g_upcb, udp_conn_rx, (void *)0);
    DebugMsg("UDP initialized to %d.\n",ptrs->report);
}

void test_send_udp(){
   
    struct ip_addr  serverIp;
    IP4_ADDR(&serverIp,192,168,1,150);

    u16_t port;
    port = 3327;
    struct udp_pcb * pcb;
    pcb = udp_new();
    pcb->ttl = UDP_TTL;
    udp_bind(pcb, IP_ADDR_ANY, port);
    udp_recv(pcb, udp_conn_rx, NULL);
    struct pbuf *p;
    char msg[]="request";

    //Allocate packet buffer
    p = pbuf_alloc(PBUF_TRANSPORT,sizeof(msg),PBUF_RAM);
    memcpy (p->payload, msg, sizeof(msg));
    udp_sendto(pcb, p, &serverIp, port);
    pbuf_free(p); //De-allocate packet buffer
}