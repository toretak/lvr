#ifndef CRC_H_
#define CRC_H_

extern unsigned short   crc_tab[256];

extern void init_crc_tab( void );
//extern unsigned short update_crc( unsigned short crc, char data ); 
inline unsigned short update_crc( unsigned short crc,unsigned char data ) {
    unsigned short tmp, short_c;
    short_c = 0x00ff & (unsigned short) data;
    tmp =  crc       ^ short_c;
    crc = (crc >> 8) ^ crc_tab[tmp & 0x00ff];
    return crc;
}  

#endif /*CRC_H_*/
