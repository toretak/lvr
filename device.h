
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

//#define TRX_IO_IRQ               GPIO_PORTD_BASE , GPIO_PIN_4
//#define TRX_IRQ_PERIPHERAL 	 SYSCTL_PERIPH_GPIOD
//#define TRX_PORT_IRQ	     	 INT_GPIOD

//---------------------------------------------------------------

// AT86RF231 Pinout
#define TRX231_PORT_BASE         GPIO_PORTD_BASE
#define TRX231_PORT_IRQ	     	 INT_GPIOD
#define TRX231_PERIPHERAL 	 SYSCTL_PERIPH_GPIOD
#define TRX231_SEL               GPIO_PIN_5
#define TRX231_IRQ               GPIO_PIN_4
#define TRX231_RST               GPIO_PIN_6
#define TRX231_SLP               GPIO_PIN_7

//---------------------------------------------------------------

// AT86RF212 Pinout

#define TRX212_PERIPHERAL 	SYSCTL_PERIPH_GPIOC //all using peripherals
#define TRX212_PORT_IRQ	     	INT_GPIOC

#define TRX212_IO_SEL           GPIO_PORTC_BASE , GPIO_PIN_4
#define TRX212_IO_IRQ           GPIO_PORTC_BASE , GPIO_PIN_5
#define TRX212_IO_RST           GPIO_PORTC_BASE , GPIO_PIN_7
#define TRX212_IO_SLP           GPIO_PORTC_BASE , GPIO_PIN_6

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
#define SOFTEEPROM_START	0x3E000	//!< Start of SoftEEPROM location in flash
#define SOFTEEPROM_END		0x3F000	//!< End of SoftEEPROM location in flash
//#define SOFTEEPROM_END		0x0003FFFF	//!< End of SoftEEPROM location in flash
#define SOFTEEPROM_SIZE         0x400

//*****************************************************************************
typedef enum {RUNNING, STOPPED, ERROR } tDeviceState;
typedef struct channelSettings
{
	int enabled;
	int baudRate;
	int parity;
	int dataBits;
	int stopBit;
} tChannelSettings;

typedef struct macList
{
	unsigned char macaddr[8];
} tMacList;

typedef struct deviceSettings 
{
	tDeviceState state;
	unsigned char ipaddr[4];
	unsigned char nmask[4];
	unsigned char gw[4];
	unsigned char macaddr[8];
        int      dhcpOn;
        int      setIpConfig;
	tChannelSettings channelSettings[8];
	tMacList	*macFilter;
} tDeviceSettings;

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************

extern unsigned long g_PacketCount;
extern unsigned long g_PacketDrop;

extern unsigned long get_time(void);

extern void DebugMsg(const char *str, ...);

extern void ledgreen_pinset(tBoolean val);

extern int Settings_Write(tDeviceSettings *sett);

//*****************************************************************************
#endif /*DEVICE_H_*/
