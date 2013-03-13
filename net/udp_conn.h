/* 
 * File:   udp_conn.h
 * Author: toretak
 *
 * Created on 12. březen 2013, 15:51
 */

#ifndef UDP_CONN_H
#define	UDP_CONN_H


// STELARISWARE
#include "lwip/opt.h"
#include "device.h"


void udpConnTx(u8_t *data, u16_t len);
void udpConnInit(tDeviceSettings *ptrs);
void test_send_udp();
//extern void zep_send_test(void);
#endif	/* TCP_CONN_H */

