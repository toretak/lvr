//*****************************************************************************
//
//! @file
//! @brief Ethernet wrapper
//
//*****************************************************************************

// STELARISWARE

#include "inc/hw_ethernet.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/ethernet.h"
#include "driverlib/gpio.h"
#include "driverlib/flash.h"
#include "utils/ustdlib.h"
#include "utils/lwiplib.h"
#include "utils/locator.h"
// PROJECT INCLUDES
#include "ethernetwrapper.h"

#define IP_ADDR(a,b,c,d) (((u32_t)((a) & 0xff) << 24) | \
                          ((u32_t)((b) & 0xff) << 16) | \
                          ((u32_t)((c) & 0xff) << 8) | \
                           (u32_t)((d) & 0xff))
                           

void IpStackInit(tDeviceSettings  *device)
{  
	
    if(device->dhcpOn==1){
      DebugMsg("DHCP enabled");
      lwIPInit((const unsigned char *)&device->macaddr, 0, 0, 0, IPADDR_USE_DHCP);
    }else{
      lwIPInit((const unsigned char *)&device->macaddr, 
			  IP_ADDR(device->ipaddr[0],device->ipaddr[1], device->ipaddr[2], device->ipaddr[3]),
			  IP_ADDR(device->nmask[0], device->nmask[1], device->nmask[2], device->nmask[3]),
			  IP_ADDR(device->gw[0], device->gw[1], device->gw[2], device->gw[3]),
			  IPADDR_USE_STATIC);
    }

    // DHCP service
    // Setup the device locator service.
    //
    LocatorInit();
    LocatorMACAddrSet((unsigned char *)&device->macaddr);
    LocatorAppTitleSet("EK-LM3S8962 enet_lwip");
}

void EthernetPeriphInit(void)
{
    unsigned long ulTemp;
	
    //
    // Enable and Reset the Ethernet Controller.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ETH);
    SysCtlPeripheralReset(SYSCTL_PERIPH_ETH);

    //
    // Enable Port F for Ethernet LEDs.
    //  LED0        Bit 3   Output
    //  LED1        Bit 2   Output
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    GPIOPinTypeEthernetLED(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3);
    
    
    //
    // Intialize the Ethernet Controller and disable all Ethernet Controller
    // interrupt sources.
    //
    EthernetIntDisable(ETH_BASE, (ETH_INT_PHY | ETH_INT_MDIO | ETH_INT_RXER |
                       ETH_INT_RXOF | ETH_INT_TX | ETH_INT_TXER | ETH_INT_RX));
    ulTemp = EthernetIntStatus(ETH_BASE, false);
    EthernetIntClear(ETH_BASE, ulTemp);

    //
    // Initialize the Ethernet Controller for operation.
    //
    EthernetInitExpClk(ETH_BASE, SysCtlClockGet());

    //
    // Configure the Ethernet Controller for normal operation.
    // - Full Duplex
    // - TX CRC Auto Generation
    // - TX Padding Enabled
    //
    EthernetConfigSet(ETH_BASE, (ETH_CFG_TX_DPLXEN | ETH_CFG_TX_CRCEN |
                                 ETH_CFG_TX_PADEN));
	
	 //
    // Wait for the link to become active.
    // 
    DebugMsg("Waiting for conection cable...\n");
    ulTemp = EthernetPHYRead(ETH_BASE, PHY_MR1);
    while((ulTemp & 0x0004) == 0)
    {
        ulTemp = EthernetPHYRead(ETH_BASE, PHY_MR1);
    }
    DebugMsg("Link Established\n");
    
	
    //
    // Enable the Ethernet controller
    //
    EthernetEnable(ETH_BASE);
        
        
}


void EthernetSetInternalMacAddr(unsigned char pucMACArray[])
{	
    pucMACArray[0] = 0xff;
    pucMACArray[1] = 0xff;
    pucMACArray[2] = 0xff;
    pucMACArray[3] = 0xff;
    pucMACArray[4] = 0xff;
    pucMACArray[5] = 0xff;	
}
