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
    DebugMsg("UDP recv\n");
    if(p == NULL)
	    return;
    
    //PROCESS INCOMMING PACKET
    pbuf_free(p);
    DebugMsg("Recieved UDP message: %s from %d.%d.%d.%d:%d\n",p->payload,ip4_addr1(addr),ip4_addr2(addr),ip4_addr3(addr),ip4_addr4(addr),port);
}
/* UDP transmit ............................................................*/
void udpConnTx(char *data, u16_t len) {
    err_t ret;
    if (ptrDeviceSettings->report != 0 && g_upcb != NULL) {
	DebugMsg("Sending UDP message %s to %d.%d.%d.%d:%d\n",data,ptrDeviceSettings->reipaddr[0],ptrDeviceSettings->reipaddr[1],ptrDeviceSettings->reipaddr[2],ptrDeviceSettings->reipaddr[3],ptrDeviceSettings->report);
        struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
	memcpy(p->payload, data, len);
	DebugMsg("Messsage in buffer: %s \n",p->payload);
	ret = udp_sendto(g_upcb, p,(struct ip_addr*)&ptrDeviceSettings->reipaddr,ptrDeviceSettings->report);
	if(ret!=ERR_OK)
	{
	    DebugMsg("Error UDP packet %d\n",ret);
	    ledgreen_pinset(1);
	}
	pbuf_free(p);
    }
}
/* UDP initialization ......................................................*/
void udpConnInit(tDeviceSettings *ptrs) {
    ptrDeviceSettings = ptrs;
    err_t ret;
    g_upcb = udp_new();
    if(g_upcb == NULL){
	DebugMsg("udp_new fail\n");
    }
    g_upcb->ttl = UDP_TTL;
    ret = udp_bind(g_upcb, /*(struct ip_addr*)&ptrs->reipaddr*/IP_ADDR_ANY, ptrs->report);
    if(ret != ERR_OK){
	DebugMsg("UDP bind fail\n");
    }
    udp_recv(g_upcb, udp_conn_rx, (void *)0);
    DebugMsg("UDP initialized to %d.\n",ptrs->report);
}

void test_send_udp(){
    err_t ret;
    struct ip_addr  serverIp;
    IP4_ADDR(&serverIp,192,168,1,150);

    u16_t port;
    port = 3327;
    struct udp_pcb * pcb;
    pcb = udp_new();
    if(pcb == NULL){
	DebugMsg("UDP_NEW fail\n");
    }
    pcb->ttl = UDP_TTL;
    ret = udp_bind(pcb, IP_ADDR_ANY, port);
    if(ret != ERR_OK){
	DebugMsg("UDP bind fail\n");
    }
    udp_recv(pcb, udp_conn_rx, NULL);
    struct pbuf *p;
    char msg[]="request";

    //Allocate packet buffer
    p = pbuf_alloc(PBUF_TRANSPORT,sizeof(msg),PBUF_RAM);
    memcpy (p->payload, msg, sizeof(msg));
    udp_sendto(pcb, p, &serverIp, port);
    pbuf_free(p); //De-allocate packet buffer
}