
// MarlinBridge.cpp

/*
	RasPi as a SPI slave (it's about Pi 1) :
	https://offis.github.io/raspi-directhw/group__spisl.html#details
*/

#include "MarlinBridge.h"
#include "Errors.h"
#include "Demo.h"

#include <stdio.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <wiringPi.h>
#include <string.h>

CMarlinBridge marlinBridge;
int CMarlinBridge::spiFileDesc;
bool CMarlinBridge::spiError = false;

// bmpOLED initialized with Marlin Info Screen
uint8_t CMarlinBridge::spiBuffer[SPI_FRAME_SIZE] = { 0 };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// opens SPI1 ; if fails, switches to demo mode

void CMarlinBridge::open()
{
	//if (settings[DEMO_MODE].val)
	//	return;

	if ((spiFileDesc = ::open("/dev/spidev1.0", O_RDWR)) < 0)
	{
		spiError = true;
		//exitError(ERROR_SPI_OPEN, __FILE__, __LINE__);
		return;
	}

	unsigned spiSpeedHz = settings[SPI_SPEED_MHZ].val * 1000000;

	if (ioctl(spiFileDesc, SPI_IOC_WR_MAX_SPEED_HZ, &spiSpeedHz) != 0)
	{
		::close(spiFileDesc);
		//exitError(ERROR_SPI_IOCTL, __FILE__, __LINE__);

		spiError = true;

		return;
	}

	spiError = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// reads a Marlin UI frame

bool CMarlinBridge::read()
{
	if(settings[DEMO_MODE].val || spiError)
		return true;
	
	if (::read(spiFileDesc, spiBuffer, SPI_FRAME_SIZE) != SPI_FRAME_SIZE)
	{
		::close(spiFileDesc);
		//exitError(ERROR_SPI_READ, __FILE__, __LINE__);
		//return false; // not in sync, etc.

		spiError = true;
		return true;
	}

	spiError = false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// closes SPI1

void CMarlinBridge::close()
{
	if (settings[DEMO_MODE].val || spiError)
		return;
	
	if (::close(spiFileDesc) < 0)
	{
		//exitError(ERROR_SPI_CLOSE, __FILE__, __LINE__);
		spiError = true;
		
		return;
	}

	spiError = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// animates two demo Marlin UI screen captures

void CMarlinBridge::swapDemoBmp()
{
	static bool b = true;
	memcpy(spiBuffer, (b = !b) ? bmpDemo0 : bmpDemo1, SPI_FRAME_SIZE);
}
