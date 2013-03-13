#include "utils/uartstdio.h"
#include "lwip/ip_addr.h"
#include "lwip/debug.h"
#include "lwip/stats.h"

#include "lwip/tcp.h"
#include <string.h>

//
#include "../device.h"
#include "../fpga/fpga_defs.h"
#include "../fpga/FPGA_IO.h"

tDeviceSettings *ptrDeviceSettings;

static void close_conn(struct tcp_pcb *pcb, struct tcp_conn_app_state *st) {
    err_t err;

    tcp_arg(pcb, NULL);
    tcp_sent(pcb, NULL);
    tcp_recv(pcb, NULL);
    if(st){
      mem_free(st);
    }
    err = tcp_close(pcb);
    if (err != ERR_OK) {

    }
}

static err_t tcp_send(struct tcp_pcb *pcb, const void *data, u16_t len) {
    return tcp_write(pcb, data, len, TCP_WRITE_FLAG_COPY);
}

static err_t tcp_conn_poll(void *arg, struct tcp_pcb *pcb) {
    struct tcp_conn_app_state * appState;
    appState = arg;
    if ((appState == NULL) && (pcb->state == ESTABLISHED)) {
      tcp_abort(pcb);
      return ERR_ABRT;
    }
    if (pcb->state == ESTABLISHED) {
	DebugMsg("Client poll\n");
        int message_sent_counter = appState->sent_messages;
        int cycles = tcp_output_counter - message_sent_counter;
        if (cycles < 0) {
            cycles += OUTPUT_TCP_BUFFER_SIZE;
        }
        DebugMsg("tcp counter %d, cycles %d\n",tcp_output_counter, cycles);
        for (int i = 0; i < cycles; i++) {
            if (message_sent_counter >= OUTPUT_TCP_BUFFER_SIZE) {
                message_sent_counter = 0;
            }
            DebugMsg("Sending message: %s\n",tcp_output_buffer[message_sent_counter].TCP_frame);
            tcp_send(pcb, tcp_output_buffer[message_sent_counter].TCP_frame, (u16_t) tcp_output_buffer[message_sent_counter].TCP_frame_length);
            message_sent_counter++;

        }
        appState->sent_messages = message_sent_counter;
        //tcp_abort(pcb);
    }else if(pcb->state == CLOSED || pcb->state == CLOSE_WAIT || pcb->state == CLOSING){
	close_conn(pcb, appState);
    }

    return ERR_OK;
}


static void conn_err(void *arg, err_t err) {

    LWIP_UNUSED_ARG(err);

}

static err_t tcp_conn_recv(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err) {
    char *data;
    char *payload;
    tcp_frame_header *header;
    short int data_length = 0;
    if ((err == ERR_OK) && (p != NULL)) {

        /* Inform TCP that we have taken the data. */
        tcp_recved(pcb, p->tot_len);
        payload = p->payload;
        header = (tcp_frame_header *)payload;
        data = &payload[5];
        data_length = ((header->data_length_hi) << 8) | header->data_length_lo;
        if(p->len > 5 /*&& data_length + 5 == p->len*/){
            

         DebugMsg("Request:\n%s\n", data);

            if (header->channel_number < 8) {
		volatile int i;
		//if(get_channel_ready(header->channel_number)){
                    for (i = 0; i < data_length; i++) {
			// send data only if channel is open and TX is not full
                        //if(!get_channel_ready(header->channel_number)){
                        //    break;
                        //}
                        //while (!get_channel_ready_TX(header->channel_number));
                        //send_data_TX(header->channel_number, data[i]);
                        //if(out_flag == 0){
                            output_buffer[i] = data[i];
                        //}
                    }
                    output_buffer_length = data_length;
                    out_flag = 1;
                //} else {
                //    tcp_send(pcb, "ch n rdy",8);
                //}
            } else {
                    //TODO
                    // channel no. is out of range (max. 8 channels)
                tcp_send(pcb, "ch out of range",15);
            }
        } else {
            //incomming nonsense
            tcp_send(pcb, "sorry I don't understand\n", 26);
        }
    }

    if ((err == ERR_OK) && (p == NULL)) {
        close_conn(pcb, arg);
    }
    pbuf_free(p);
    return ERR_OK;
}

static err_t
tcp_conn_accept(void *arg, struct tcp_pcb *pcb, err_t err) {

    struct tcp_conn_app_state *st;

    LWIP_UNUSED_ARG(arg);
    LWIP_UNUSED_ARG(err);
    /* Tell TCP that we wish to be informed of incoming data by a call
       to the http_recv() function. */
//    ((struct tcp_conn_app_state *) arg)->sent_messages = 0;

    st = (struct tcp_conn_app_state *)mem_malloc(sizeof(struct tcp_conn_app_state));
    if (st == NULL) {
      DebugMsg("tcp_conn: Out of memory!\n");
      return ERR_MEM;
    }
    //MAC FILTER 
  //neeed to find out, if MAC filter enabled
    if(ptrDeviceSettings->mfEnabled == 1){
      DebugMsg("MAC filter enabled\n");
      struct netif tnetif;
      struct eth_addr * adr;
      struct ip_addr * ip;
      DebugMsg("Trying to find mac for ip: %d.%d.%d.%d\n",ip4_addr1(&pcb->remote_ip),ip4_addr2(&pcb->remote_ip),ip4_addr3(&pcb->remote_ip),ip4_addr4(&pcb->remote_ip));
      int find = etharp_find_addr(&tnetif, &pcb->remote_ip, &adr, &ip);
      if(find == ERR_OK){
	  DebugMsg("Found mac: %02X:%02X:%02X:%02X:%02X:%02X :]\n",(*adr).addr[0],(*adr).addr[1],(*adr).addr[2],(*adr).addr[3],(*adr).addr[4],(*adr).addr[5]);
	  // look for addres in mac filter
	  int result = 0;
	  for(int i=0;i<ptrDeviceSettings->macFilterListLen;i++){
	    result += eth_addr_cmp(&((ptrDeviceSettings->macFilter[i]).macaddr),adr);
	    DebugMsg("searching in mac list (index %d) : %d\n",i, result);
	  }
	  if(result == 0) find = -1;
      }
      if(find != ERR_OK){
	DebugMsg("MAC not found in white list, disconnecting..");
	close_conn(pcb, st);
	return ERR_OK;
      }
    }
  //MAC FILTER ENDS  
    st->sent_messages = 0;
    tcp_arg(pcb, st);

    tcp_recv(pcb, tcp_conn_recv);

    tcp_err(pcb, conn_err);

    tcp_poll(pcb, tcp_conn_poll, 4);
    return ERR_OK;
}

void tcpConnInit(tDeviceSettings *sett) {
    struct tcp_pcb *pcb;
    ptrDeviceSettings = sett;
    DebugMsg("tcp %d init\n",sett->port);

    pcb = tcp_new();
    tcp_bind(pcb, IP_ADDR_ANY, sett->port);
    pcb = tcp_listen(pcb);
    tcp_accept(pcb, tcp_conn_accept);

    DebugMsg("tcp %d init finished\n",sett->port);

}


