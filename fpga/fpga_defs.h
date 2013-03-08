/*
 * fpga_defs.h
 *
 *  Created on: 8.3.2013
 *      Author: toretak
 */

#ifndef FPGA_DEFS_H_
#define FPGA_DEFS_H_

#define UNIQUE_ADAPTER_ID 0x66

// the AFTN message size including start and stop characters
#define MAX_MESSAGE_SIZE 500

//Default FPGA channel settings

#define DEFAULT_CONTROL_REG     0x19
#define DEFAULT_FRAME_SEL       0x34
#define FPGA_DELAY              25

//*****************************************************************************
// Definitions of data pins
//*****************************************************************************
#define DTB_PERIPHER SYSCTL_PERIPH_GPIOA
#define DTB_PORT GPIO_PORTA_BASE

#define DB0 GPIO_PIN_0
#define DB0_PORT GPIO_PORTA_BASE

#define DB1 GPIO_PIN_1
#define DB1_PORT GPIO_PORTA_BASE

#define DB2 GPIO_PIN_2
#define DB2_PORT GPIO_PORTA_BASE

#define DB3 GPIO_PIN_3
#define DB3_PORT GPIO_PORTA_BASE

#define DB4 GPIO_PIN_4
#define DB4_PORT GPIO_PORTA_BASE

#define DB5 GPIO_PIN_5
#define DB5_PORT GPIO_PORTA_BASE

#define DB6 GPIO_PIN_6
#define DB6_PORT GPIO_PORTA_BASE

#define DB7 GPIO_PIN_7
#define DB7_PORT GPIO_PORTA_BASE

//*****************************************************************************
// Definitions of control pins
//*****************************************************************************
#define CTB_PERIPHER SYSCTL_PERIPH_GPIOD
#define CTB_PORT GPIO_PORTD_BASE

#define AD GPIO_PIN_0
#define AD_PORT GPIO_PORTD_BASE

#define EN GPIO_PIN_1
#define EN_PORT GPIO_PORTD_BASE

#define RESET_SIGNAL GPIO_PIN_2
#define RESET_SIGNAL_PORT GPIO_PORTD_BASE


#define OUTPUT_TCP_BUFFER_SIZE  10
#endif


struct tcp_frm{
    // buffer, where the TCP frame is stored during sending TCP frames to all data clients
    char TCP_frame[MAX_MESSAGE_SIZE + 5];
    int TCP_frame_length;
} tcp_output_buffer[OUTPUT_TCP_BUFFER_SIZE];

int tcp_output_counter;
char output_buffer[MAX_MESSAGE_SIZE];
int output_buffer_length;
int out_flag;

typedef struct{
    char packet_type;
    char adapter_id;
    char channel_number;
    char data_length_hi;
    char data_length_lo;
}tcp_frame_header;

struct tcp_conn_app_state{
    int sent_messages;
};