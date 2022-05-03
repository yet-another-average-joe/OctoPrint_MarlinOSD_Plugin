#pragma once

// interface with the STM32 Marlin bridge
// static class (one instance only)

#include "Settings.h"

class CMarlinBridge // static class
{
	static int spiFileDesc;

public:

	static unsigned char spiBuffer[SPI_FRAME_SIZE]; // bitmap, 1 bit per pixel, big endian
	static bool spiError;

	static void open();			// opens SPI1 ; is fails, switches to demo mode
	static bool read();			// reads one frame to spiBuffer (Marlin UI bitmap, SPI_FRAME_SIZE bytes )
	static void close();		// closes SPI1
	static void swapDemoBmp();	// animates two demo Marlin UI screen captures
};

