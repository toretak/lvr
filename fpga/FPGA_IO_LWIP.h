/*
 * FPGA_IO_LWIP.h
 *
 *  Created on: 25.04.2013
 *      Author: Bayer, Brejcha
 */

#ifndef FPGA_IO_LWIP_H_
#define FPGA_IO_LWIP_H_

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

// FPGA error type
typedef enum
{
  NO_ERROR, ERROR
} t_error_FPGA;

void FPGA_interface_init(void);
void FPGA_channel_init(const tDeviceSettings *sett);
t_error_FPGA configure_FPGA_channel(const char channel, const tChannelSettings channel_settings);
void send_data(char data);
void send_address(char addr);
char read_input(void);
t_error_FPGA send_message_to_FPGA(const char channel, const char *message_array, const int message_length);
char check_for_new_message(void);
t_error_FPGA TCP_frame_load_new_message(const char channel, const tDeviceSettings *sett, char TCP_frame[MAX_MESSAGE_SIZE + 5], int *TCP_frame_length);
void clear_error_flag(const char channel, const char control_reg);
void clear_new_message_flag(const char channel, const char control_reg);
void get_adapter_status(tChannelSettings *channel_settings);
void TCP_frame_load_adapter_states(const tDeviceSettings *sett, char *TCP_frame, int *TCP_frame_length);

#endif /* FPGA_IO_H_ */

//*****************************************************************************
// macros
//*****************************************************************************
#define create_address(channel, register_type) (((channel << 3) & CHANNEL_MASK) | register_type)
