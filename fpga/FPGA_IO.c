/*
 * FPGA_IO.c
 *
 *  Created on: 21.11.2012
 *      Author: Bayeriste
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
#include "globalDefinitions.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "stdbool.h"
#include "FPGA_IO.h"
#include "httpserver_raw/httpd.h"
#include "EEPROM_IO.h"

//****************************************************************************************************************************
// Global variables for this module
//****************************************************************************************************************************
#ifdef ONE_CHANNEL
char control_reg_storage;					//for control register
char frame_reg_storage;						//for frame register
#else
char control_reg_storage[8];					//for control registers
char frame_reg_storage[8];						//for frame registers
#endif

//****************************************************************************************************************************
// Initiate interface FPGA with MCU
//****************************************************************************************************************************
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

//****************************************************************************************************************************
// Resets a FPGA's channel by the reset signal and waits for its ready bit to be set.
// When in 8-channel mode, all channels are being reset by the one reset signal and it is sufficient to check for readiness of
// a single channel.
//****************************************************************************************************************************
void FPGA_init()
{
	// set reset signal high and low to reset and return to normal operation
	GPIOPinWrite(RESET_SIGNAL_PORT, RESET_SIGNAL, 0xff);
        GPIOPinWrite(RESET_SIGNAL_PORT, RESET_SIGNAL, 0x00);
        

	// configure channels according to previous saved settings or defaults if not saved yet. Settings are saved also to RAM mirror variables.
	char control_reg_temp, frame_sel_temp;
#ifdef ONE_CHANNEL
        control_reg_temp = 0x19;
        frame_sel_temp = 0x34;
	//EEPROM_retrieve_channel_settings(0, &control_reg_temp, &frame_sel_temp);
	send_control_reg_raw(0, control_reg_temp);
	send_frame_sel_raw(0, frame_sel_temp);
	clear_error_flag(0);
	clear_new_message_flag(0);
        while (!get_channel_ready(0)) {
        }
#else
	volatile int i;
	for (i = 0; i < 8; i++) {
		EEPROM_retrieve_channel_settings((char)i, &control_reg_temp, &frame_sel_temp);
		send_control_reg_raw(i, control_reg_temp);
		send_frame_sel_raw((char)i, frame_sel_temp);
		clear_error_flag(i);
		clear_new_message_flag(i);
		// TODO  get_ready???
	}
#endif

/*
	//enable and set default 9600 bauds for all channels
	for (i = 0; i < 8; i++)
	{
		send_control_reg(i, false, true, true, true, 9, 0xff);
		send_frame_sel(i, 3, 0, 0, 0xff);
		while (!get_channel_ready(i)) {
		}
	}
*/
}

//****************************************************************************************************************************
// Sends control register and frame selection, saves settings to EEPROM.
// Not for CEF and CNMF clearing (to clear flags use send_control_reg).
//****************************************************************************************************************************
void send_channel_settings(char channel, bool reset, bool enable_channel, char baud_rate, char control_reg_mask,
		char data_bits, char stop_bits, char parity, char frame_sel_mask) {

	// send settings to control register and frame selection register of FPGA
	send_control_reg(channel, reset, enable_channel, false, false, baud_rate, control_reg_mask);
	send_frame_sel(channel, data_bits, stop_bits, parity, frame_sel_mask);

	// save settings to EEPROM
	// create control_reg part
	char temp_control_reg, temp_control_reg_storage, temp_frame_reg, temp_frame_reg_storage;

	temp_control_reg = ((RESET * reset) | (ENABLE_CHANNEL * enable_channel) | baud_rate);
	temp_control_reg &= control_reg_mask;

	// mask the storage of the register (previous values)
#ifdef ONE_CHANNEL
	temp_control_reg_storage = control_reg_storage & ~(control_reg_mask);
#else
	temp_control_reg_storage = control_reg_storage[channel] & ~(control_reg_mask);
#endif

	// combine both previous masked register and the new masked changed value
	temp_control_reg_storage |= temp_control_reg;

	// create frame_sel part
	// choose the data length
	switch (data_bits) {
	case 0:
		data_bits = 0;
		break;
	case 1:
		data_bits = DATA_BITS0;
		break;
	case 2:
		data_bits = DATA_BITS1;
		break;
	case 3:
		data_bits = (DATA_BITS0 | DATA_BITS1);
		break;
	default:
		//unsupported selection
		break;
	}

	switch (stop_bits) {
	case 0:
		stop_bits = STOP_BITS0;
		break;
	case 1:
		stop_bits = STOP_BITS1;
		break;
	case 2:
		stop_bits = (STOP_BITS0 | STOP_BITS1);
		break;
	default:
		//unsupported selection;
		break;
	}

	switch (parity) {
	case 0:
		parity = 0;
		break;
	case 1:
		parity = PARITY_ENABLE;
		break;
	case 2:
		parity = (PARITY_ENABLE | PARITY_ODD);
		break;
	default:
		//unsupported selection;
		break;
	}

	// mask the register (pick new settings)
	temp_frame_reg = (data_bits | stop_bits | parity);
	temp_frame_reg &= frame_sel_mask;

	// mask the storage of the register (previous values)
#ifdef ONE_CHANNEL
	temp_frame_reg_storage = frame_reg_storage & ~(frame_sel_mask);
#else
	temp_frame_reg_storage = frame_reg_storage[channel] & ~(frame_sel_mask);
#endif

	// combine both previous masked register and the new masked changed value
	temp_frame_reg_storage |= temp_frame_reg;

	// save both registers to EEPROM
	EEPROM_save_channel_settings(channel, temp_control_reg_storage, temp_frame_reg_storage);
}

//****************************************************************************************************************************
// TODO 1/8 channel version - should not be possible to set another channel when using one_channel version
// send CONTROL_REG
// channel (0 - 7), baud_rate according to FPGA specifications (0 - 14)
// baud_rate: 0 ~ 50 BAUDS, 1 ~ 75, 2 ~ 110, 3 ~ 150, 4 ~ 300, 5 ~ 600, 6 ~ 1200, 7 ~ 2400, 8 ~ 4800,
// 9 ~ 9600, 10 ~ 19200, 11 ~ 28800, 12 ~ 38400, 13 ~ 57600, 14 ~ 115200 BAUDS
// mask - the bit that is going to be changed
//****************************************************************************************************************************
void send_control_reg(char channel, bool reset, bool enable_channel, bool clear_error_flag, bool clear_new_message_flag, char baud_rate, char mask)
{
	char temp_control_reg;
	char temp_control_reg_storage;

	// check for proper settings
#ifdef ONE_CHANNEL
	if ((channel != 0) || (baud_rate > 14)) return;
#else
	if ((channel > 7) || (baud_rate > 14)) return;
#endif

	send_address(channel << 3 | CONTROL);

	// mask the register (pick new settings)
	temp_control_reg = ((RESET * reset) | (ENABLE_CHANNEL * enable_channel) | baud_rate | (CEF * clear_error_flag) | (CNMF * clear_new_message_flag));
	temp_control_reg &= mask;

	// mask the storage of the register (previous values)
#ifdef ONE_CHANNEL
	temp_control_reg_storage = control_reg_storage & ~(mask);
#else
	temp_control_reg_storage = control_reg_storage[channel] & ~(mask);
#endif

	// combine both previous masked register and the new masked changed value
	temp_control_reg_storage |= temp_control_reg;

	//sending data
	send_data(temp_control_reg_storage);

	// don't save CEF and CNMF bit(CEF and CNMF are one-time flags)
#ifdef ONE_CHANNEL
	control_reg_storage = temp_control_reg_storage & ~(CEF | CNMF);
#else
	control_reg_storage[channel] = temp_control_reg_storage & ~(CEF | CNMF);
#endif

}

// send raw data to control_reg (unformatted as it is stored in the control register of the FPGA)
// used for initialization from EEPROM after reset
// do not use to clear CEF and CNMF (flags)
void send_control_reg_raw(char channel, char control_reg) {
	// check for proper settings of the channel
#ifdef ONE_CHANNEL
	if (channel != 0) return;
#else
	if (channel > 7) return;
#endif

	send_address(channel << 3 | CONTROL);
	send_data(control_reg);

	// save settings to mirror variable, used for variable initialization
#ifdef ONE_CHANNEL
	control_reg_storage = control_reg;
#else
	control_reg_storage[channel] = control_reg;
#endif
}

//****************************************************************************************************************************
// TODO 1/8 channel version - should not be possible to set another channel when using one_channel version
// send FRAME_SEL
// channel (0 - 7), data_bits (0 - 3), stop_bits (0 - 2), parity (0 - 2)
// data_bits: 0 ~ 5 bits, 1 ~ 6 bits, 2 ~ 7 bits, 3 ~ 8 bits
// stop_bits: 0 ~ 1 bit, 1 ~ 1.5 bits, 2 ~ 2 bits
// parity: 0 ~ none, 1 ~ even, 2 ~ odd
// mask - the bit that is going to be changed
//********************************************************************************* *******************************************

void send_frame_sel(char channel, char data_bits, char stop_bits, char parity, char mask)
{
	char temp_frame_reg;
	char temp_frame_reg_storage;

	// check for proper settings
#ifdef ONE_CHANNEL
	if ((channel != 0) || (data_bits > 3) || (stop_bits > 2) || (parity > 2)) return;
#else
	if ((channel > 7) || (data_bits > 3) || (stop_bits > 2) || (parity > 2)) return;
#endif

	send_address(channel << 3 | FRAME);

	// choose the data length
	switch (data_bits) {
	case 0:
		data_bits = 0;
		break;
	case 1:
		data_bits = DATA_BITS0;
		break;
	case 2:
		data_bits = DATA_BITS1;
		break;
	case 3:
		data_bits = (DATA_BITS0 | DATA_BITS1);
		break;
	default:
		//unsupported selection
		break;
	}

	switch (stop_bits) {
	case 0:
		stop_bits = STOP_BITS0;
		break;
	case 1:
		stop_bits = STOP_BITS1;
		break;
	case 2:
		stop_bits = (STOP_BITS0 | STOP_BITS1);
		break;
	default:
		//unsupported selection;
		break;
	}

	switch (parity) {
	case 0:
		parity = 0;
		break;
	case 1:
		parity = PARITY_ENABLE;
		break;
	case 2:
		parity = (PARITY_ENABLE | PARITY_ODD);
		break;
	default:
		//unsupported selection;
		break;
	}

	// mask the register (pick new settings)
	temp_frame_reg = (data_bits | stop_bits | parity);
	temp_frame_reg &= mask;

	// mask the storage of the register (previous values)
#ifdef ONE_CHANNEL
	temp_frame_reg_storage = frame_reg_storage & ~(mask);
#else
	temp_frame_reg_storage = frame_reg_storage[channel] & ~(mask);
#endif

	// combine both previous masked register and the new masked changed value
	temp_frame_reg_storage |= temp_frame_reg;

	send_data(temp_frame_reg_storage);

	// save new settings to RAM variable
#ifdef ONE_CHANNEL
	frame_reg_storage = temp_frame_reg_storage;
#else
	frame_reg_storage[channel] = temp_frame_reg_storage;
#endif
}

// send raw data to frame_sel (unformatted as it is stored in the frame selection register of the FPGA)
// used for initialization from EEPROM after reset
void send_frame_sel_raw(char channel, char frame_sel) {
	// check for proper settings of the channel
#ifdef ONE_CHANNEL
	if (channel != 0) return;
#else
	if (channel > 7) return;
#endif

	send_address(channel << 3 | FRAME);

	send_data(frame_sel);

	// save settings to mirror variable, used for variable initialization
#ifdef ONE_CHANNEL
	frame_reg_storage = frame_sel;
#else
	frame_reg_storage[channel] = frame_sel;
#endif
}

//****************************************************************************************************************************
// Data port sending
//****************************************************************************************************************************
void send_data(char data)
{
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

//****************************************************************************************************************************
// Data port reading
//****************************************************************************************************************************
char read_input(void)
{
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


//****************************************************************************************************************************
// sends an address byte, channel and desired registers must be set manually
//****************************************************************************************************************************
void send_address(char addr)
{
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

//****************************************************************************************************************************
// returns true if channel is ready and not full, false if channel is not ready or full
// used when trying to write to TX
//****************************************************************************************************************************
bool get_channel_ready_TX(char channel)
{
	send_address(create_address(channel, STATUS));
	if((read_input() & (TX_FULL))) {
	//not ready
		return false;
	}
	return true;
}


//****************************************************************************************************************************
// return true if channel is ready
// used for configuration commands
//****************************************************************************************************************************
bool get_channel_ready(char channel)
{
	send_address(create_address(channel, STATUS));

	if (read_input() & READY) return true;
	//if (read_input() & 0x01) return true;

	return false;
}

//****************************************************************************************************************************
// check if channel has new data
//****************************************************************************************************************************
bool get_channel_new_data(char channel)
{
	send_address(create_address(channel, STATUS));

	char c = read_input();
	if(c & RX_EMPTY) {
		// no data in RX to be read
		return false;
	}

	return true;

}

//****************************************************************************************************************************
// check if new message is ready (all channels show the Message Ready Flag in one byte)
// returns a channel number with Message Ready Flag set (0 - 7) or 8 (no new message)
//****************************************************************************************************************************
char check_for_new_message(void)
{
	send_address(create_address(0, MSG_READY));

	char c = read_input();

	// Looks at the LSB and checks for set. If bit not set, then shift bits and check again... return the number of shifts.
	volatile int i;
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

//****************************************************************************************************************************
// Load new message, available in FPGA's FIFO buffer, into TCP_FRAME.
// Create header of the frame.
//****************************************************************************************************************************
void TCP_frame_load_new_message(char channel, char TCP_frame[MAX_MESSAGE_SIZE + 5], int *TCP_frame_length) {

	// no frame header test
	char c;
	volatile int i;

	send_address(create_address(channel, RX));

	c = read_input();
	*TCP_frame_length = 0;

#ifdef FPGA_TO_TCP_THROUGHPUT
	for (i = 0; (i < MAX_MESSAGE_SIZE) && (c != 0); i++) {
		// the address is the same
		TCP_frame[i] = c;
		*TCP_frame_length = i + 1;
		c = read_input();
	}
#else

	// Read one by one chars from FPGA's FIFO. Leave 5 empty bytes for a header.
	for (i = 0; (i < MAX_MESSAGE_SIZE) && (c != 0); i++) {
		// the address is the same
		TCP_frame[i + 5] = c;
		*TCP_frame_length = i + 6;
		c = read_input();
	}

	TCP_frame[0] = 0;
	TCP_frame[1] = UNIQUE_ADAPTER_ID;
	TCP_frame[2] = channel;
	TCP_frame[3] = (char)(*TCP_frame_length >> 8);
	TCP_frame[4] = (char)*TCP_frame_length;
#endif

	clear_new_message_flag(channel);
}

//****************************************************************************************************************************
// check channel for errors
// 0 ~ no error, 1 ~ parity error, 2 ~ frame error, 3 ~ both errors
//****************************************************************************************************************************
char get_channel_errors(char channel)
{
	send_address(create_address(channel, STATUS));

	char error_flag_register = 0;
	char input = read_input();

	if (input & PARITY_ERROR) {
		error_flag_register += 1;
	}
	if (input & FRAME_ERROR) {
		error_flag_register += 2;
	}

	return error_flag_register;
}

//****************************************************************************************************************************
// sends CEF
//****************************************************************************************************************************
void clear_error_flag(char channel)
{
	send_address(create_address(channel, CONTROL));

	// CEF bit masks the other -> only channel and CEF settings are processed
	send_control_reg(channel, false, true, true, true, 9, CEF);
}

//****************************************************************************************************************************
// sends CNMF
//****************************************************************************************************************************
void clear_new_message_flag(char channel)
{
	send_address(create_address(channel, CNMF));

	// CNMF bit masks the other -> only channel and CEF settings are processed
	send_control_reg(channel, false, true, true, true, 9, CNMF);
}

//****************************************************************************************************************************
// send data to TX of a specified channel
// channel [0x00:0x07]
//****************************************************************************************************************************
void send_data_TX(char channel, char data)
{
	//address of the TX register of the given channel
	send_address(create_address(channel, TX));
	send_data(data);
}

//****************************************************************************************************************************
// get char from RX register of the FPGA
//****************************************************************************************************************************
char get_data_RX(char channel)
{
	// address of the RX register
	send_address(create_address(channel, RX));
	return read_input();
}


//****************************************************************************************************************************
// get adapter status registers
//****************************************************************************************************************************
char get_adapter_status(char channel)
{
	// address of the RX register
	send_address(create_address(channel, ADAPTER_STATUS));
	return read_input();
}

//****************************************************************************************************************************
// get adapter status registers of all channels, combine them into one TCP frame
//****************************************************************************************************************************
void TCP_frame_load_adapter_states(char *TCP_frame, int *TCP_frame_length) {


	// create TCP frame header
	TCP_frame[0] = 0;
	TCP_frame[1] = UNIQUE_ADAPTER_ID;
	TCP_frame[2] = 0xff;	// all channels in one TCP frame
	TCP_frame[3] = 1;
	TCP_frame[4] = 1;

	// get channel's adapter status
#ifdef ONE_CHANNEL
	TCP_frame[5] = get_adapter_status(0);
	*TCP_frame_length = 6;
#else
	volatile int i;
	for (i = 0 ; i < 8; i++) {
		TCP_frame[i + 5] = get_adapter_status(i);
	}
	*TCP_frame_length = 13;
#endif

}

