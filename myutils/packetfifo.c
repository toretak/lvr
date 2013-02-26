//*****************************************************************************
//
//! @file
//! @brief Buffer FIFO for packets
//
//*****************************************************************************

#include "packetfifo.h"

// buffer for packets
TtrxPacket 	 packetBuf[FIFO_SIZE];
// array for paketBuf state (empty,busy,available)
TpacketState packetBufAvailable[FIFO_SIZE];

// cursor for reading from fifo
unsigned char  rIndex;

// cursor for writing to fifo
unsigned char  wIndex;


void packetFifoInit(void)
{
	unsigned char i;
	
	rIndex = EMPTY_INDEX;
	wIndex = 0;
	
	for(i=0;i<FIFO_SIZE;i++)
		packetBufAvailable[i] = EMPTY;	
}

TtrxPacket* packetFifoAdd(void)
{
	unsigned char i;
	
	i=wIndex;
	while(packetBufAvailable[i] !=	EMPTY)
	{
		unsigned char nextwIndex;
		nextwIndex = (i + 1) % FIFO_SIZE;
		if(nextwIndex == rIndex)
			return NULL; // full fifo
		i = nextwIndex;
	}	
	// critical
	wIndex = i;
	packetBufAvailable[wIndex] = BUSY;
	//
	return &packetBuf[wIndex];
	
}

tBoolean packetFifoAddDone(TtrxPacket *trxPacketPtr)
{
	unsigned char i;

	if(rIndex == EMPTY_INDEX) 
		rIndex = 0;
	
	i = rIndex;
	
	while(trxPacketPtr != &packetBuf[i])
	{
		i = (i + 1) % FIFO_SIZE;
		if(i==rIndex)
			return false; // wrong input pointer
	}
	//	
	packetBufAvailable[i] = AVAILABLE;
	wIndex = (wIndex + 1) % FIFO_SIZE;
	//	
	return true;
}

tBoolean packetFifoAddFailed(TtrxPacket *trxPacketPtr)
{
	unsigned char i;

	if(rIndex == EMPTY_INDEX) 
		rIndex = 0;
	
	i = rIndex;
	
	while(trxPacketPtr != &packetBuf[i])
	{
		i = (i + 1) % FIFO_SIZE;
		if(i==rIndex)
			return false; // wrong input pointer
	}
	//	
	packetBufAvailable[i] = EMPTY;
	//	
	return true;
}


TtrxPacket* packetFifoGet(void)
{
	// We need to send ordered packets, therefore we wait until current packet is ready
	if(packetBufAvailable[rIndex] == AVAILABLE)
	{
		packetBufAvailable[rIndex] = BUSY;
		return	&packetBuf[rIndex];
	}
	else
		return NULL; // empty fifo
}

void packetFifoGetDone(void)
{
	packetBufAvailable[rIndex] = EMPTY;
	rIndex = (rIndex+1) % FIFO_SIZE;	
}
