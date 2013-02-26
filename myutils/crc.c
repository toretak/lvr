//*****************************************************************************
//
//! @brief CRC
//
//*****************************************************************************


//
// CRC computation
// 
// source : http://www.lammertbies.nl/comm/software/index.html
//          http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html

#include "crc.h"

unsigned short   crc_tab[256];

#define P_KERMIT    0x8408

// fill crc tab
void init_crc_tab( void ) {
    int i, j;
    unsigned short crc, c;
    for (i=0; i<256; i++) {
        crc = 0;
        c   = (unsigned short) i;
        for (j=0; j<8; j++) {
            if ( (crc ^ c) & 0x0001 ) crc = ( crc >> 1 ) ^ P_KERMIT;
            else                      crc =   crc >> 1;
            c = c >> 1;
        }
        crc_tab[i] = crc;
    }
}  

//implementation crc by table
//before was inline
//inline unsigned short update_crc( unsigned short crc, char data ) {
//    unsigned short tmp, short_c;
//    short_c = 0x00ff & (unsigned short) data;
//    tmp =  crc       ^ short_c;
//    crc = (crc >> 8) ^ crc_tab[tmp & 0xff];
//    return crc;
//}  

// implementation crc without table
#define lo8(x) ((x)&0xff) 
#define hi8(x) ((x)>>8) 
//before was inline
unsigned short crc_ccitt_update (unsigned short crc, unsigned char data)
{
        data ^= lo8 (crc);
        data ^= data << 4;
        return ((((unsigned short)data << 8) | hi8 (crc)) ^ (unsigned char)(data >> 4)^ ((unsigned short)data << 3));
}

