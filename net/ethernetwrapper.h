#ifndef ETHERNETWRAPPER_H_
#define ETHERNETWRAPPER_H_

#include "../device.h"

void IpStackInit(tDeviceSettings  *sniffer);
void EthernetPeriphInit(void);
void EthernetSetInternalMacAddr(unsigned char pucMACArray[]);

#endif /*ETHERNETWRAPPER_H_*/
