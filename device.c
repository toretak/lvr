//*****************************************************************************
//
// enet_io.c - I/O control via a web server.
//
// Copyright (c) 2007-2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 9453 of the EK-LM3S8962 Firmware Package.
//
//*****************************************************************************


// COMPILER
#include <stdarg.h>
#include "string.h"
// STELARISWARE
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/timer.h"

#include "utils/softeeprom.h"
// PROJECT INCLUDES
#include "device.h"
#include "net/http_conf.h"
#include "net/ethernetwrapper.h"
#include "myutils/crc.h"
#include "myutils/uartstdio.h"

//*****************************************************************************
//
//!
//! ../../../tools/bin/makefsfile -i fs -o io_fsdata.h -r -h -q
//
//*****************************************************************************

//*****************************************************************************
//
// Defines for setting up the system clock.
//
//*****************************************************************************
#define SYSTICKHZ               100
#define SYSTICKMS               (1000 / SYSTICKHZ)
#define SYSTICKUS               (1000000 / SYSTICKHZ)
#define SYSTICKNS               (1000000000 / SYSTICKHZ)

//*****************************************************************************
//
// A set of flags.  The flag bits are defined as follows:
//
//     0 -> An indicator that a SysTick interrupt has occurred.
//
//*****************************************************************************
#define FLAG_SYSTICK            0
static volatile unsigned long g_ulFlags;

#define DEBUG


//*****************************************************************************
//
//! Global variables
//
//*****************************************************************************
    unsigned long g_TimeCounter;	


    tDeviceSettings  deviceSettings;
    
//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif


void DebugInit(void)
{
#ifdef DEBUG
	#define BAUD_RATE 9600

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
	UARTStdioInitExpClk(0,BAUD_RATE);
#endif	
}
//*****************************************************************************
//
//! Debug messages
//!
//! Description 1
//!
//! \param str is constant format of output
//!
//! \param ... is any other variables
//
//*****************************************************************************

void DebugMsg(const char* str, ...)
{
#ifdef DEBUG
    va_list vaArgP;
    va_start(vaArgP, str);
	UARTvprintf(str,vaArgP);
	va_end(vaArgP);
#endif
}
void ledgreen_pinset(tBoolean val)
{
  if(val)
  	GPIOPinWrite(LED_PORT_BASE, LED_GREEN, 0xFF);
  else
    GPIOPinWrite(LED_PORT_BASE, LED_GREEN, 0x00);  	
}

//*****************************************************************************
void LED_init(void)
{
    SysCtlPeripheralEnable(LED_PERIPHERAL); 	
    GPIOPinTypeGPIOOutput(LED_PORT_BASE, LED_GREEN); 	
    ledgreen_pinset(0);  

}

//*****************************************************************************
//
// Write
//
//*****************************************************************************
int Settings_Write(tDeviceSettings *sett)
{
     
    unsigned char *ptr_sett=(unsigned char*)sett;  
    unsigned short id;
    unsigned short crc=0x0000;   
               
    
    DebugMsg("Size of Settings=%d\n",sizeof(tDeviceSettings));  
    
    sett->setIpConfig=0;
      
    //
    // write deviceSettings settings to emulation EEPROM
    //
    for(id=0;id<(sizeof(tDeviceSettings));id++)  //div 2 WARNING !!!!!!!!!!!!!!!
    {
      if(SoftEEPROMWrite(id, ptr_sett[id])!=0)
      {
         DebugMsg("SoftEEPROM write failed on %d\n",id);
         return 0;
         
      }
      crc = update_crc(crc, ptr_sett[id]);
    } 
    
    if(SoftEEPROMWrite(id, crc)!=0)
    {    
      DebugMsg("SoftEEPROM write failed on %d\n",id); 
      return 0;
    }
    
    DebugMsg("Settings_Write OK\n");
      
    return 1;
}

//*****************************************************************************
//
// Default
//
//*****************************************************************************

int Settings_Default(tDeviceSettings *sett)
{
	sett->dhcpOn=0;
        
	sett->ipaddr[0]=192;
	sett->ipaddr[1]=168;
	sett->ipaddr[2]=1;
	sett->ipaddr[3]=64; 

        sett->port = 3327;
        sett->pr = 0;   //TCP
        
	sett->nmask[0]=255;
	sett->nmask[1]=255;
	sett->nmask[2]=255;
	sett->nmask[3]=0;
	
	sett->gw[0]=192;
	sett->gw[1]=168;
	sett->gw[2]=1;
	sett->gw[3]=1;
	
        sett->mfEnabled = 0;                            //disable mac filter
        sett->setIpConfig=0;                            //unset changing IP flag
        sett->macFilterListLen = 0;        //clear mac filter list
        //
        //Save Default setting to Flash memory
        //
        if(!Settings_Write(sett))
          return 0;
        
        return 1;

}

//*****************************************************************************
//
// Init
//
//*****************************************************************************
int Settings_Read(tDeviceSettings *sett)
{
    
    unsigned char *ptr_sett=(unsigned char*)sett;  
    unsigned char id;
    unsigned short data,crc=0x0000; 
    tBoolean found;
    
  
    //
    // read deviceSettings from emulation EEPROM
    // 

    
    for(id=0;id<(sizeof(tDeviceSettings));id++) 
    {
      
      
      if(SoftEEPROMRead(id, &data, &found)!=0)
      {
         DebugMsg("!!SoftEEPROM read failed on %d\n",id);
         return 0;          
      }
      
      //if read is succesfull than check founded data
      if(found==1)
      {
        ptr_sett[id]=data;        
        crc = update_crc(crc, ptr_sett[id]);
      }
      else
      {
        DebugMsg("!!SoftEEPROM was not found on %d\n",id);
        return 0;
      }
      
    } 
    
      if(SoftEEPROMRead(id, &data, &found)!=0)
      {
         DebugMsg("!!SoftEEPROM read failed on %d\n",id);
         return 0;          
      }
      
      //if read is succesfull than check founded data
      if(found==false)
      {
        DebugMsg("!!SoftEEPROM was not found on %d\n",id);
        return 0;
      }
    
    DebugMsg("SoftEEPROM CRC: %04x %04x\n",data, crc);

    if((data!=crc) || (data==0) || (crc==0))
    {
      DebugMsg("WW:SoftEEPROM CRC fail, Setting Default\n");
      if(!Settings_Default(sett))
        return 0;
     
    
    }
        
    
    DebugMsg("State=%d, IP=%d.%d.%d.%d\n",sett->state,   
             sett->ipaddr[0], sett->ipaddr[1],sett->ipaddr[2],sett->ipaddr[3]);
    
    DebugMsg("Mask=%d.%d.%d.%d, GW=%d.%d.%d.%d\n",sett->nmask[0],
             sett->nmask[1],sett->nmask[2],sett->nmask[3],
             sett->gw[0],sett->gw[1],sett->gw[2],sett->gw[3]);

    DebugMsg("Setting Read OK\n");
    return 1;
}

//*****************************************************************************
int Settings_Init(void)
{  
              
    //
    // initialize SoftEEPROM functionality and setup EEPROM area
    //
    if (SoftEEPROMInit(SOFTEEPROM_START, SOFTEEPROM_END, SOFTEEPROM_SIZE) != 0)
    {    	
      return 0;
    
    }
    
    return 1;
}
//*****************************************************************************
//
// The interrupt handler for the SysTick interrupt.
//
//*****************************************************************************
void SysTickIntHandler(void)
{
    //
    // Indicate that a SysTick interrupt has occurred.
    //
    HWREGBITW(&g_ulFlags, FLAG_SYSTICK) = 1;

    //
    // Call the lwIP timer handler.
    //
    lwIPTimer(SYSTICKMS);
}

void Button_init(void)
{
      
    SysCtlPeripheralEnable(BUTTON_PERIPHERAL);
    GPIODirModeSet(BUTTON_IO, GPIO_DIR_MODE_IN);
    GPIOPadConfigSet(BUTTON_IO, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);


}

int main(void)
{
    unsigned long *pucTemp;
    unsigned long *ulIp, *ulMask, *ulGw;
    //
    // Set the clocking to run directly from the crystal.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_8MHZ);

    //
    // Configure SysTick for a periodic interrupt.
    //
    SysTickPeriodSet(SysCtlClockGet() / SYSTICKHZ);
    SysTickEnable();
    SysTickIntEnable();

    LED_init(); 
   
    //
    // Init Ports, Irq etc.
    //
    DebugInit(); 
    
    //
    // Initialization emulation EEPROM on FLASH memory
    //
    if(Settings_Init() == 0)
    {
      DebugMsg("!!!FATAL cano't initialize soft EEPROM\n");
      while(1);
    
    }
    
    //
    // Read previous configuration from FLASH memory
    //
    DebugMsg("\nloading device settings\n");
    if(Settings_Read(&deviceSettings) == 0)
    {
      DebugMsg("\t\t\t\t[!!]");
      DebugMsg("\nfallback: loading defaults");
      Settings_Default(&deviceSettings);
      
    } else {
      DebugMsg("\t\t\t\t[OK]"); 
    }
    
    //
    // Set default settings if Reset button has been pressed.
    //
    if(GPIOPinRead(BUTTON_IO) == 0)
    {
      DebugMsg("Button was push and Settings is Default\n");
      Settings_Default(&deviceSettings);
    }


        
    DebugMsg("\nstarting ... \n");
    DebugMsg("System clock = %d Hz\n", SysCtlClockGet());
	
  
    //
    // Configure Ethernet pins and mode
    //
    EthernetPeriphInit(); 
    
    //
    // Enable processor interrupts.
    //
    IntMasterEnable();

    //
    // Set Ethernet MAC address
    //
    //EthernetSetInternalMacAddr((unsigned char *)&deviceSettings.macaddr);
   
    //
    // Initialize the lwIP library
    //                       
    IpStackInit(&deviceSettings);
    DebugMsg("\nstarting stack library \t\t\t\t[OK]");
    //
    // Initialize httpd server.
    //                    
    HttpdInit(&deviceSettings);  
    DebugMsg("\nstarting HTTPD \t\t\t\t\t[OK]");
    //
    // Loop forever.  All the work is done in interrupt handlers.
    //
    while(1)
    {
/*
        if(deviceSettings.setIpConfig==1)
        { 
             ulIp = (unsigned long *)&deviceSettings.ipaddr;
             ulMask = (unsigned long *)&deviceSettings.nmask;  
             ulGw = (unsigned long *)&deviceSettings.gw;

             //
             // Wait a while
             //
             delay(2000000);

             if(deviceSettings.dhcpOn==0)
               lwIPNetworkConfigChange(htonl(*ulIp), htonl(*ulMask), htonl(*ulGw), IPADDR_USE_STATIC);
             else
             {
                   //
                   // If DHCP has already been started, we need to clear the IPs and
                   // switch to static.  This forces the LWIP to get new IP address
                   // and retry the DHCP connection.
                   //
                   lwIPNetworkConfigChange(0, 0, 0, IPADDR_USE_STATIC);

                   //
                   //Restart the DHCP connection.
                   //
                   lwIPNetworkConfigChange(0, 0, 0, IPADDR_USE_DHCP);


                   DebugMsg("Getting IP address from DHCP...\n");

                   pucTemp = (unsigned long *)&deviceSettings.ipaddr;

                   //
                   //  Wait until DHCP assign IP address
                   //
                   while((*pucTemp=lwIPLocalIPAddrGet())==0)
                   {
                      delay(1000000);                 
                   }

                   pucTemp = (unsigned long *)&deviceSettings.nmask; 
                   *pucTemp=lwIPLocalNetMaskGet();   
                   pucTemp = (unsigned long *)&deviceSettings.gw;
                   *pucTemp=lwIPLocalGWAddrGet();
             }

             deviceSettings.setIpConfig=0;
        }
*/
    }
}
