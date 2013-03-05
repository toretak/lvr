
#ifndef DEVICE_H_
#define DEVICE_H_

#include "inc/hw_types.h"

//*****************************************************************************
//
//! \defgroup sniffer_api Sniffer
//! @{
//
//*****************************************************************************


//*****************************************************************************

#define DEBUG

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
//#define SOFTEEPROM_START	0x3E000	//!< Start of SoftEEPROM location in flash
//#define SOFTEEPROM_END		0x3F000	//!< End of SoftEEPROM location in flash
//#define SOFTEEPROM_SIZE         0x400
#define SOFTEEPROM_START	0x00010000
#define SOFTEEPROM_END		0x00011000
#define SOFTEEPROM_SIZE				0x00000800
//#define SOFTEEPROM_END		0x0003FFFF	//!< End of SoftEEPROM location in flash

#define MAC_LIST_SZIZE          5

//*****************************************************************************
typedef enum {RUNNING, STOPPED, ERROR } tDeviceState;
typedef enum {OK, FE, PE, FP } tChannelState;
typedef struct channelSettings
{
	int enabled;
	int baudRate;
	int parity;
	int dataBits;
	int stopBit;
        tChannelState state;        
} tChannelSettings;

typedef struct macList
{
	unsigned char macaddr[8];
} tMacList;

typedef struct deviceSettings 
{
	tDeviceState state;
	unsigned char ipaddr[4];
	unsigned short port;
	unsigned char nmask[4];
	unsigned char gw[4];
	unsigned char macaddr[8];
        int pr;                         //protocol : 0 - TCP, 1 - UDP
	unsigned char reipaddr[4];
	unsigned short report;
	tChannelSettings channelSettings[8];
	tMacList macFilter[MAC_LIST_SZIZE];
        int macFilterListLen;
        int mfEnabled;
        int dhcpOn;
	int setIpConfig;
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
