#ifndef PACKETFIFO_H_
#define PACKETFIFO_H_

#include "inc/hw_types.h"
#include "../radio/packet.h"

#define FIFO_SIZE 32 // max 255 (0-254)
#define EMPTY_INDEX 0xFF

typedef enum {EMPTY, BUSY, AVAILABLE } TpacketState;

#ifndef NULL
#define NULL                    ((void *)0)
#endif

extern void packetFifoInit(void);
extern TtrxPacket* packetFifoAdd(void);
extern tBoolean packetFifoAddDone(TtrxPacket *trxPacketPtr);
extern tBoolean packetFifoAddFailed(TtrxPacket *trxPacketPtr);
extern TtrxPacket* packetFifoGet(void);
extern void packetFifoGetDone(void);

#endif /*PACKETFIFO_H_*/
