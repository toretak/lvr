/*
 * FPGA_IO.h
 *
 *  Created on: 22.11.2012
 *      Author: Administrator
 */

#ifndef FPGA_IO_H_
#define FPGA_IO_H_

#include "stdbool.h"

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

//*****************************************************************************
// addresses of registers
//*****************************************************************************
// FPGA_IO definitions
// address byte, register type
#define CONTROL	0x00
#define FRAME	0x01
#define STATUS 	0x02			// software status (flags)
#define RX 		0x03
#define TX 		0x04
#define ADAPTER_STATUS	0x05	// hardware status (open pair, ...)
#define MSG_READY 0x40

//*****************************************************************************
// masks of bits
//*****************************************************************************
// control register
#define BAUD0			0x01
#define BAUD1			0x02
#define BAUD2			0x04
#define BAUD3			0x08
#define ENABLE_CHANNEL	0x10
#define RESET			0x20
#define CEF				0x40	// Clear Error Flag
#define CNMF			0x80	// Clear New Message Flag (clears Full RX Error (when RX is full and flushed) and New Message Flag)
// frame select register
#define PARITY_ENABLE	0x01
#define PARITY_ODD		0x02
#define STOP_BITS0		0x04
#define STOP_BITS1		0x08
#define DATA_BITS0		0x10
#define DATA_BITS1		0x20
// status register
#define TX_FULL			0x01
#define TX_EMPTY		0x02
#define RX_FULL			0x04
#define RX_EMPTY  		0x08
#define	READY			0x10
#define	FRAME_ERROR		0x20
#define PARITY_ERROR	0x40
//	address creation
#define CHANNEL_MASK	0x38

//*****************************************************************************
// declarations
//*****************************************************************************
void FPGA_interface_init(void);
void FPGA_init();
void send_channel_settings(char channel, bool reset, bool enable_channel, char baud_rate, char control_reg_mask, char data_bits, char stop_bits, char parity, char frame_sel_mask);
void send_control_reg(char channel, bool reset, bool enable_channel, bool clear_error_flag, bool clear_new_message_flag, char baud_rate, char mask);
void send_control_reg_raw(char channel, char control_reg);
void send_frame_sel(char channel, char data_bits, char stop_bits, char parity, char mask);
void send_frame_sel_raw(char channel, char frame_sel);
void send_data(char data);
char read_input(void);
void send_address(char addr);
bool get_channel_ready_TX(char channel);
bool get_channel_ready(char channel);
bool get_channel_new_data(char channel);
char check_for_new_message(void);
void TCP_frame_load_new_message(char channel, char *TCP_frame, int *TCP_frame_length);
char get_channel_errors(char channel);
void clear_error_flag(char channel);
void clear_new_message_flag(char channel);
void send_data_TX(char channel, char data);
char get_data_RX(char channel);
char get_adapter_status(char channel);
void TCP_frame_load_adapter_states(char *TCP_frame, int *TCP_frame_length);

//*****************************************************************************
// macros
//*****************************************************************************
#define create_address(channel, register_type) (((channel << 3) & CHANNEL_MASK) | register_type)

#endif /* FPGA_IO_H_ */
