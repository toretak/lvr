/*
 * Copyright (c) 2001, Swedish Institute of Computer Science.
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 3. Neither the name of the Institute nor the names of its contributors 
 *    may be used to endorse or promote products derived from this software 
 *    without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
 * SUCH DAMAGE. 
 * 
 * arp.c
 *                     
 * Author : Adam Dunkels <adam@sics.se>                               
 *
 * CHANGELOG: this file has been modified by Sergio Perez Alcañiz <serpeal@upvnet.upv.es> 
 *            Departamento de Informática de Sistemas y Computadores          
 *            Universidad Politécnica de Valencia                             
 *            Valencia (Spain)    
 *            Date: April 2003                                          
 *  
 */

#include "lwip/inet.h"
#include "utils/arp.h"
#include "lwip/ip.h"



/*-----------------------------------------------------------------------------------*/
u8_t get_u8_t_from_char(char c,int low){

  if(low){
    /* a-z */
    if((c >= 97) && (c <= 122)) return (c-87);
    /* A-Z */
    else if((c >= 65) && (c <= 90)) return (c-55);
    /* 0-9 */
    else if((c >= 48) && (c <= 57)) return (c-48);
  }else{ /* !low */
    /* a-z */
    if((c >= 97) && (c <= 122)) return ((c-87)<<4);
    /* A-Z */
    else if((c >= 65) && (c <= 90)) return ((c-55)<<4);
    /* 0-9 */
    else if((c >= 48) && (c <= 57)) return ((c-48)<<4);
  }
  return -1;
}

/*-----------------------------------------------------------------------------------*/
unsigned char get_char_from_u8_t(u8_t value){
  if(value < 10){
    return (unsigned char)(value + 48); //48 is the ascii code for '0'
  }else{
    return (unsigned char)(value + 55); //65 would be the ascii code for 'A'
  }
}

/*-----------------------------------------------------------------------------------*/
void string2mac(struct eth_addr *mac, char *name){
  int i,hop=0;

  for(i=0; i<6; i++){
    mac->addr[i] = get_u8_t_from_char(name[hop],0) + get_u8_t_from_char(name[hop+1],1);
    hop+=3;
  }
}

/*-----------------------------------------------------------------------------------*/
void mac2string(struct eth_addr *mac, char *name){
  int i,hop=0;
  u8_t aux;

  for(i=0; i<5; i++){
    aux = mac->addr[i];
    name[hop] = get_char_from_u8_t((aux & 0xf0)>>4);
    name[hop+1] = get_char_from_u8_t((aux & 0x0f));
    name[hop+2] = 0x3a; //0x3a is the code ascii (in hex) for ':'
    hop+=3;
  }
  name[hop] = get_char_from_u8_t((mac->addr[5] & 0xf0)>>4);
  name[hop+1] = get_char_from_u8_t((mac->addr[5] & 0x0f));
}
