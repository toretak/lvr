/*
 * FPGA_IO_LWIP.c
 *
 *  Created on: 25.04.2013
 *      Author: Bayer, Brejcha
 */


/* address byte: [7:6] reserved, [5:3] channel number, [2:0] register selection
 * register selection [0000 0XXX]
 * 000 - control register - write to this register to setup BAUD rate, enable the given channel and reset the output circuitry
 * 001 - frame selection - write to this register to change output communication settings such as number of data bits, stop bits, parity checking
 * 010 - status register - read from this register to check for errors, readiness, RX and TX full and empty flags
 * 011 - RX register - the receive register
 * 100 - TX register - the transmit register
 * channel selection [00XX X000] - select channel number
 * reserved [XX00 0000]
 */

#include "fpga_defs.h"
#include "../device.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "FPGA_IO_LWIP.h"
#include "httpserver_raw/httpd.h"

//******************************************************************************
//! Initiates FPGA - MCU interface.
//! This method initiates the interface between FPGA and MCU (output/input pins...).
void FPGA_interface_init(void) {
	// write 0 to EN and AD bits before they are set as outputs so they start as low outputs
	GPIOPinWrite(AD_PORT, AD, 0x00);
	GPIOPinWrite(EN_PORT, EN, 0x00);
	GPIOPinTypeGPIOOutput(AD_PORT, AD);
	GPIOPinTypeGPIOOutput(EN_PORT, EN);

	// set re-init signal to low - signal that can make problems on the target board
	GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, 0x00);
	GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_0);

	// reset signal should be set 1 to reset FPGA before operation
	GPIOPinWrite(RESET_SIGNAL_PORT, RESET_SIGNAL, 0x00);
	GPIOPinTypeGPIOOutput(RESET_SIGNAL_PORT, RESET_SIGNAL);

	// sets the pins to be weak pull-up (input and output)
	GPIOPadConfigSet(DB0_PORT, DB0, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD_WPU);
	GPIOPadConfigSet(DB1_PORT, DB1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD_WPU);
	GPIOPadConfigSet(DB2_PORT, DB2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD_WPU);
	GPIOPadConfigSet(DB3_PORT, DB3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD_WPU);
	GPIOPadConfigSet(DB4_PORT, DB4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD_WPU);
	GPIOPadConfigSet(DB5_PORT, DB5, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD_WPU);
	GPIOPadConfigSet(DB6_PORT, DB6, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD_WPU);
	GPIOPadConfigSet(DB7_PORT, DB7, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD_WPU);
}

//******************************************************************************
//! Initiates FPGA channels.
//! This method initiates channels of the FPGA and resets all error flags.
//! \param *sett the data structure containig channelSettings
void FPGA_channel_init(const tDeviceSettings *sett) {
	// set reset signal high and low to reset and return to normal operation
	GPIOPinWrite(RESET_SIGNAL_PORT, RESET_SIGNAL, 0xff);
	GPIOPinWrite(RESET_SIGNAL_PORT, RESET_SIGNAL, 0x00);

	// configure channels according to previous saved settings or defaults if not saved yet. Settings are saved also to RAM mirror variables.
#ifdef ONE_CHANNEL	
  configure_FPGA_channel(0, sett->channelSettings[0]);
  clear_new_message_flag(0, sett->channelSettings[0].controlReg);
  clear_error_flag(0, sett->channelSettings[0].frameSelReg);
#else
	volatile int i;
	for (i = 0; i < 8; i++) {
		configure_FPGA_channel(i, sett->channelSettings[i]));
		clear_new_message_flag(i, sett->channelSettings[i].controlReg);
  	clear_error_flag(i, sett->channelSettings[i].frameSelReg);
	}
#endif
}

//******************************************************************************
//! Configures FPGA channel.
//! This method sends channel settings to FPGA taken from the tChannelSettings data structure.
//! \param channel The channel to configure.
//! \param channel_settings Channel settings that will be sent.
//! \return NO_ERROR if an existing channel was chosen, ERROR if a wrong channel was chosen.
t_error_FPGA configure_FPGA_channel(const char channel, const tChannelSettings channel_settings) {
	// check for a proper channel number
#ifdef ONE_CHANNEL
	if (channel != 0) return ERROR;
#else
	if (channel > 7) return ERROR;
#endif

	send_address(create_address(channel, CONTROL));
	send_data(channel_settings.controlReg & ~(CEF | CNMF)); // do not clear flags
	send_address(create_address(channel, FRAME));
	send_data(channel_settings.frameSelReg);
	return NO_ERROR;
}

//******************************************************************************
//! Sends data to FPGA.
//! \param data The data to be sent.
void send_data(char data) {
	volatile int delay;

	GPIOPinTypeGPIOOutputOD(DTB_PORT, 0xff);	// port is set as output

	// clear AD bit = data
	GPIOPinWrite(AD_PORT, AD, 0x00);

	// data on port
	GPIOPinWrite(DTB_PORT, 0xff, data);

  // set EN bit (tells FPGA to read)
  GPIOPinWrite(EN_PORT, EN, 0xff);

  // Wait a bit, 500 ns requested (4 cycles worked for 8 MHz main clock)
  for (delay = 0; delay < FPGA_DELAY; delay++);

  // clear EN bit
  GPIOPinWrite(EN_PORT, EN, 0x00);
}

//******************************************************************************
//! Sends address to FPGA.
//! \addr The address od the register data will be sent to.
void send_address(char addr) {
	volatile int delay;

	GPIOPinTypeGPIOOutputOD(DTB_PORT, 0xff);	// port is set as output

	// set AD bit = address, FPGA sets high Z to its previous output pins
	GPIOPinWrite(AD_PORT, AD, 0xff);

	// data on port
	GPIOPinWrite(DTB_PORT, 0xff, addr);

	// set EN bit (tells FPGA to read)
	GPIOPinWrite(EN_PORT, EN, 0xff);

	// Wait a bit, 500 ns requested (4 cycles worked for 8 MHz main clock)
	for (delay = 0; delay < FPGA_DELAY; delay++);

	// clear EN bit
	GPIOPinWrite(EN_PORT, EN, 0x00);
}

//******************************************************************************
//! Read data sent by FPGA.
//! \return The ouptut byte of the FPGA.
char read_input(void) {
	char received_char = 0;
	volatile int delay;

	GPIOPinTypeGPIOInput(DTB_PORT, 0xff);		// port is set as input

	// clear AD bit = data
	GPIOPinWrite(AD_PORT, AD, 0x00);

	// set EN bit (tells FPGA that ARM is reading)
	GPIOPinWrite(EN_PORT, EN, 0xff);

	// Added because of error when reading DB0 (to fast with 50 MHz clock)
	for (delay = 0; delay < FPGA_DELAY; delay++);

	// reading
	received_char = GPIOPinRead(DTB_PORT, 0xff);

	// clear EN bit
	GPIOPinWrite(EN_PORT, EN, 0x00);

	return received_char;
}


//******************************************************************************
//! Sends a message to the FPGA.
//! \param channel The channel the message will be sent to.
//! \param *message_array The message-byte-array, that will be sent to FPGA.
//! \param message_length Length of the message-byte-array.
//! return NO_ERROR if an existing channel was chosen, ERROR if a wrong channel was chosen.
t_error_FPGA send_message_to_FPGA(const char channel, const char *message_array, const int message_length) {
	
#ifdef ONE_CHANNEL	
	if ((message_length == 0) || (channel != 0)) return ERROR;
#else
	if ((message_length == 0) || (channel > 7)) return ERROR;
#endif
	
	send_address(create_address(channel, TX));
	
	volatile int i;
	for (i = 0; i < message_length; i++) {
		send_data(message_array[i]);
	}
	
	return NO_ERROR;
}

//******************************************************************************
//! Checks for a new available message.
//! \return The lowest channel with a message ready. If 8 is returned, there is no new message available.
//! \note Message Ready Register contains Message Ready Flags of all channels.
char check_for_new_message(void) {
	send_address(create_address(0, MSG_READY));

	char c = read_input();

	// Looks at the LSB and checks for set. If bit not set, then shift bits and check again... return the number of shifts.
	volatile char i;
	for (i = 0; i < 8; i++) {
		if (c & 1) {
			return i;
		} else {
			c = c >> 1;
		}
	}

	// no flag is set
	return 8;
}

//******************************************************************************
//! Loads a new message from a given channel of the FPGA.
//! This method loads a new message from a FPGA channel, creates a proper header.
//! \param channel The channel the message will be read from.
//! \param *channel_settings Settings of the given channel.
//! \param TCP_frame[MAX_MESSAGE_SIZE + 5] A char array, the received message will be stored to.
//! \param *TCP_frame_length Length of the char array saved to TCP_frame.
//! \return NO_ERROR if an existing channel was chosen, ERROR if a wrong channel was chosen.
//! \note tChannelSettings are needed for New Message Flag clearing.
t_error_FPGA TCP_frame_load_new_message(const char channel,const tDeviceSettings *sett, char TCP_frame[MAX_MESSAGE_SIZE + 5], int *TCP_frame_length) {

	// no frame header test
	char c;
	volatile int i;

	send_address(create_address(channel, RX));

	c = read_input();
	*TCP_frame_length = 0;

	// Read one by one chars from FPGA's FIFO. Leave 5 empty bytes for a header.
	for (i = 0; (i < MAX_MESSAGE_SIZE) && (c != 0xff); i++) {
		// the address is the same
		TCP_frame[i + 5] = c;
		*TCP_frame_length = i + 6;
		c = read_input();
	}

	TCP_frame[0] = 0;
	TCP_frame[1] = sett->device_id;
	TCP_frame[2] = channel + 1;
	TCP_frame[3] = (char)(((*TCP_frame_length) - 5) >> 8);

	TCP_frame[4] = (char)((*TCP_frame_length) - 5);

  clear_new_message_flag(channel, sett->channelSettings[((int) channel)].controlReg);
  
  return NO_ERROR;
}

//******************************************************************************
//! \Clears the Error Flag of the given FPGA channel.
//! \param channel The channel whose Error Flag will be cleared.
//! \param control_reg The channel's Control Register content.
void clear_error_flag(const char channel, const char control_reg) {
	send_address(create_address(channel, CONTROL));

	// CEF bit masks the other -> only channel and CEF settings are processed
	send_data(control_reg | CEF);
}

//******************************************************************************
//! \Clears the New Message Flag of the given FPGA channel.
//! \param channel The channel whose New Message Flag will be cleared.
//! \param control_reg The channel's Control Register content.
void clear_new_message_flag(const char channel, const char control_reg) {
	send_address(create_address(channel, CONTROL));

	// CNMF bit masks the other -> only channel and CEF settings are processed
	send_data(control_reg | CNMF);
}

//******************************************************************************
//! \Gets adapter status/states.
//! \Gets adapter status from FPGA (ONE_CHANNEL version) or adapter states from all FPGA channels.
//! \param *channel_settings An array of structures containing adapterStatusReg.
//! \note For the ONE_CHANNEL version, *channel_settings is a pointer to a single char. For the Eight Channel version, every member's adapterStatusReg of the array of structures will be filled with the read staes. 
void get_adapter_status(tChannelSettings *channel_settings) {
	// get channel's adapter status
#ifdef ONE_CHANNEL
	// address of the status register
	send_address(create_address(0, ADAPTER_STATUS));
	// save read settings to structure
	channel_settings->adapterStatusReg = read_input();
#else
	// address of the status register
	volatile int i;
	for (i = 0; i < 8; i++) {
		send_address(create_address(i, ADAPTER_STATUS));
		// save read settings to structure
		channel_settings[i].adapterStatusReg = read_input();
	}		
#endif
} 

//****************************************************************************************************************************
//! \Gets adapter status registers of all channels, combines them into one TCP frame.
//! \param TCP_frame A char array, the received message will be stored to.
//! \param Length of the char array saved to TCP_frame.
void TCP_frame_load_adapter_states(const tDeviceSettings *sett, char *TCP_frame, int *TCP_frame_length) {

	volatile int i;

	// create TCP frame header
	TCP_frame[0] = 1;
	TCP_frame[1] = sett->device_id;
#ifdef ONE_CHANNEL
	TCP_frame[2] = 0x01; // channel number
	TCP_frame[3] = 0;	// length of data (TCP_frame[5 - ...])
	TCP_frame[4] = 1;	// length of data (TCP_frame[5 - ...])
#else
	TCP_frame[2] = 0xff;	// all channels in one TCP frame
	TCP_frame[3] = 0;	// length of data (TCP_frame[5 - ...])
	TCP_frame[4] = 8;	// length of data (TCP_frame[5 - ...])
#endif

	// get channel's adapter status
#ifdef ONE_CHANNEL
	// address of the RX register
	send_address(create_address(0, ADAPTER_STATUS));
	TCP_frame[5] = read_input();
	*TCP_frame_length = 6;
#else
	for (i = 0 ; i < 8; i++) {
		send_address(create_address(i, ADAPTER_STATUS));
		TCP_frame[i + 5] = read_input();
	}
	*TCP_frame_length = 13;
#endif
}
