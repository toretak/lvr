
#ifndef DEVICE_H_
#define DEVICE_H_

#define ONE_CHANNEL
#define SIMULATION_MODE
#define DEBUGPRINT

#include "inc/hw_types.h"
#include "utils/arp.h"

//*****************************************************************************
//
//! \defgroup sniffer_api Sniffer
//! @{
//
//*****************************************************************************


//*****************************************************************************

#define FPGA_FW_VERSION "612v01"
#define MCU_FW_VERSION "602v02"
#define BUILD_DATE "2013-Feb-22"

//#define CPU_XTAL 8000000

//---------------------------------------------------------------

// LED status
#define LED_PORT_BASE	         GPIO_PORTF_BASE
#define LED_PERIPHERAL		 SYSCTL_PERIPH_GPIOF
#define LED_GREEN		 GPIO_PIN_0

//---------------------------------------------------------------

// Reset button
#define BUTTON_PERIPHERAL       SYSCTL_PERIPH_GPIOF
#define BUTTON_IO               GPIO_PORTF_BASE, GPIO_PIN_1



//*****************************************************************************
//! \name SoftEEPROM Constants
// Defines for the SoftEEPROM area.
//@{
//*****************************************************************************
#define SOFTEEPROM_START	0x30000	//!< Start of SoftEEPROM location in flash
#define SOFTEEPROM_END		0x3F000	//!< End of SoftEEPROM location in flash
#define SOFTEEPROM_SIZE         0x1000
//#define SOFTEEPROM_START	0x00020000
//#define SOFTEEPROM_END		0x00021000
//#define SOFTEEPROM_SIZE				0x00000800
//#define SOFTEEPROM_END		0x0003FFFF	//!< End of SoftEEPROM location in flash
#ifdef ONE_CHANNEL
#define CHANNELS 1
#else
#define CHANNELS 8
#endif
#define MAC_LIST_SZIZE          5

//*****************************************************************************
/**
 * controlReg
 * CNMF | CEF | RESET | ENABLE | BAUD | BAUD | BAUD | BAUD
 * 
 * - CNMF - clear new message flag
 * - CEF - clear error
 * - RESET - reset channel
 * - ENABLE - chennel enabled
 * - BAUD:
 *   0000 - 50 baud/s
 *   0001 - 75 baud/s
 *   ....
 *   1101 - 5760 baud/s
 * >=1110 - 115200 baud/s
 * 
 * 
 * frameSelReg
 * DATAWIDTH(2) | STOPBIT(2) | PAR_SEL | PAR_ENABLE
 *  
 *   00 - 5 bitu
 *   01 - 6 bitu
 *   10 - 7 bitu
 *   11 - 8 bitu
 * 
 *   00,01 - 1 stop bit
 *   10    - 1,5 stop bitu
 *   11    - 2 stop bity
 * 
 *   1 - even parity
 *   0 - odd
 * 
 */
typedef struct channelSettings
{
	char controlReg;
	char frameSelReg;
} tChannelSettings;

typedef struct macList
{
	struct eth_addr macaddr;
} tMacList;

typedef struct deviceSettings 
{
	unsigned char ipaddr[4];
	unsigned short port;
	unsigned char nmask[4];
	unsigned char gw[4];
	unsigned char macaddr[8];
        char pr;                         //protocol : 0 - TCP, 1 - UDP
	unsigned char reipaddr[4];
	unsigned short report;
	tChannelSettings channelSettings[CHANNELS];
	tMacList macFilter[MAC_LIST_SZIZE];
        char macFilterListLen;
        char mfEnabled;
        char dhcpOn;
	short setIpConfig;
	short device_id;
	short serial_number;
} tDeviceSettings;

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************


extern unsigned long get_time(void);

extern void DebugMsg(const char *str, ...);

extern void ledgreen_pinset(int val);

extern int Settings_Write(tDeviceSettings *sett);

//*****************************************************************************
#endif /*DEVICE_H_*/
