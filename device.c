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

#include "netif/etharp.h"
// PROJECT INCLUDES
#include "device.h"
#include "net/http_conf.h"
#include "net/ethernetwrapper.h"
#include "myutils/crc.h"
#include "myutils/uartstdio.h"

#include "fpga/fpga_defs.h"
#include "net/tcp_conn.h"
#include "net/udp_conn.h"
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
#ifdef DEBUGPRINT
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif


void DebugInit(void)
{
#ifdef DEBUGPRINT
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
#ifdef DEBUGPRINT
    va_list vaArgP;
    va_start(vaArgP, str);
	UARTvprintf(str,vaArgP);
	va_end(vaArgP);
#endif
}
void ledgreen_pinset(int val)
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
    DebugMsg("CRC: %04x",crc);
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
	
	sett->reipaddr[0]=192;
	sett->reipaddr[1]=168;
	sett->reipaddr[2]=1;
	sett->reipaddr[3]=150; 
	sett->report = 3327;
	
	sett->setIpConfig=0;                     //unset changing IP flag
	//sett->mfEnabled = 0;			 //disable mac filter
        //sett->macFilterListLen = 0;       	 //clear mac filter list
	
	sett->mfEnabled = 1;
        sett->macFilterListLen = 1;
        (sett->macFilter[0]).macaddr.addr[0] = 0x00;
	(sett->macFilter[0]).macaddr.addr[1] = 0x21;
	(sett->macFilter[0]).macaddr.addr[2] = 0x70;
	(sett->macFilter[0]).macaddr.addr[3] = 0xb3;
	(sett->macFilter[0]).macaddr.addr[4] = 0xce;
	(sett->macFilter[0]).macaddr.addr[5] = 0x8b;
	
        for (int i=0;i<CHANNELS;i++){
            sett->channelSettings[i].controlReg = 0x19;//0b00011001;
            sett->channelSettings[i].frameSelReg = 0xB0;//0b10110000;
        }
        
        //
        //Save Default setting to Flash memory
        //
        if(!Settings_Write(sett)){
	  return 0;
	}
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
      if(found==true)
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
        
    
    DebugMsg("IP=%d.%d.%d.%d\n", sett->ipaddr[0], sett->ipaddr[1],sett->ipaddr[2],sett->ipaddr[3]);
    
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
void Timer0IntHandler(void)
{
    // Clear the timer interrupt.
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);    

    // Read the current state of the output
    long pin_status;
    pin_status=GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_0);
    
    // Toggle Bit 0
    pin_status^=GPIO_PIN_0;
    
    // Write the result back into the GPIO Pin 0 Data Register
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_0, pin_status);

    etharp_tmr();
    
    test_send_udp();
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
    // Calculate all variant of CRC for packet checksum radio modules
    //
    init_crc_tab();
    
    //
    // Initialization emulation EEPROM on FLASH memory
    //
    if(Settings_Init() == 0)
    {
      DebugMsg("!!!FATAL can't initialize soft EEPROM\n");
      while(1);
    
    }
    
    //
    // Read previous configuration from FLASH memory
    //
    DebugMsg("\nloading device settings\n");
    if(Settings_Read(&deviceSettings) == 0)
    {
      DebugMsg("\nfallback: loading defaults");
      Settings_Default(&deviceSettings);
      
    }
    
        
    DebugMsg("\nbooting ... \n");
    DebugMsg("System clock = %d Hz\n", SysCtlClockGet());
	
  
    //
    // Configure Ethernet pins and mode
    //
    EthernetPeriphInit(); 
    
    //
    // Set Ethernet MAC address
    //
    //EthernetSetInternalMacAddr((unsigned char *)&deviceSettings.macaddr);
   
    //
    // Initialize the lwIP library
    //                       
    IpStackInit(&deviceSettings);
    DebugMsg("\nstarting stack library \t\t\t\t[OK]\n");
    //
    // Initialize httpd server.
    //                    
    HttpdInit(&deviceSettings);  
    DebugMsg("\nstarting HTTPD \t\t\t\t\t[OK]\n");
    
    //initialize fpga
    DebugMsg("\ninitializing FPGA communication \t\t");
#ifdef SIMULATION_MODE
    
    DebugMsg("[simulation mode]\n");
#else
    FPGA_interface_init();
    FPGA_init(&deviceSettings);
    DebugMsg("[OK]\n");
#endif    
    //initialize AFTN ethernet
    DebugMsg("\nstarting TCP AFTN translator\t\t\t[OK]\n");
    tcpConnInit(&deviceSettings);
    DebugMsg("\nstarting UDP AFTN translator\t\t\t[OK]\n");
    //udpConnInit(&deviceSettings);
    
        //initialize arp table
    etharp_init();
    DebugMsg("\nEthArp initialized \t\t\t\t[OK]\n");
    //enable timer for etharp_tmr
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_PER);
    // Set Timer0 period as 1/4 of the system clock, i.e. 4 interrupts per second
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet());

    // Configure the timer to generate an interrupt on time-out
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);    
    // Enable the Timer Interrupt in the NVIC
    IntEnable(INT_TIMER0A);
    // Enable the timer
    TimerEnable(TIMER0_BASE, TIMER_A);
    
    
    //
    // Set default settings if Reset button has been pressed.
    //
//     if(GPIOPinRead(BUTTON_IO) == 0)
//     {
// 	DebugMsg("Button was push and Settings is Default\n");
// 	Settings_Default(&deviceSettings);
//     }

    //
    // Enable processor interrupts.
    //
    IntMasterEnable();
    ledgreen_pinset(1);  
    //
    // Loop forever.  All the work is done in interrupt handlers.
    //
    tcp_output_counter = 0;
    while(1)
    {        
        //process tcp communication
#ifdef SIMULATION_MODE
        if(tcp_output_counter <= 0){
	    char str[] = "simulation mode";
            for(int i=0;i<sizeof(str);i++){
                tcp_output_buffer[tcp_output_counter].TCP_frame[i] = str[i];
            }
            tcp_output_buffer[tcp_output_counter].TCP_frame_length = sizeof(str);
            tcp_output_counter = 1;
        }
#else
        char channel = check_for_new_message();
        if (channel < 8) {
            TCP_frame_load_new_message(channel, tcp_output_buffer[tcp_output_counter].TCP_frame, &tcp_output_buffer[tcp_output_counter].TCP_frame_length);
            tcp_output_counter++;
            if(tcp_output_counter >= OUTPUT_TCP_BUFFER_SIZE){
                tcp_output_counter = 0;
            }
        }
        if(out_flag == 1){
	  //if(get_channel_ready(0)){
            for(int i = 0;i < output_buffer_length;i++){
                send_data_TX(0, output_buffer[i]);
            }
            out_flag = 0;
	  //}
        }
        //
#endif
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
