
/*

	MarlinBridge.cpp

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

// TODO : if needed, get raw data from MarlinOctoHat and transpose bitmap here ; could save 1 or 2 milliseconds...
//#define __TX_RAW

CMarlinBridge marlinBridge;
int CMarlinBridge::spiFileDesc;
bool CMarlinBridge::spiError = false;

// bmpOLED initialized with Marlin Info Screen
uint8_t CMarlinBridge::spiBuffer[MARLIN_BMP_SIZE] = { 0 };

#ifdef __TX_RAW

void bmpOledToLinear(uint8_t* bmpOut, uint8_t** bmpIn)

#endif // __TX_RAW

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// open SPI1 ; if fails, switches to demo mode

void CMarlinBridge::open()
{
	const char* dev = getSpiDev();

	if ((spiFileDesc = ::open(dev, O_RDWR)) < 0)
	{
		spiError = true;

		return;
	}

	unsigned spiSpeedHz = settings[SPI_SPEED].val * 1000000;

	if (ioctl(spiFileDesc, SPI_IOC_WR_MAX_SPEED_HZ, &spiSpeedHz) != 0)
	{
		::close(spiFileDesc);

		spiError = true;

		return;
	}

	spiError = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// read Marlin UI frame

bool CMarlinBridge::read()
{
	if(settings[DEMO_MODE].val || spiError)
		return true;
	
#ifdef	__TX_RAW
	if (::read(spiFileDesc, spiBuffer, MARLIN_BMP_SIZE) != MARLIN_BMP_SIZE)
	{
		::close(spiFileDesc);
		spiError = true;

		return true;
	}

	memcpy(bmpOled, spiBuffer, MARLIN_BMP_SIZE);
	bmpOledToBmpOut();		// input = bmpOled, output = bmpOut (-> spiBuffer for now !)
	memcpy(spiBuffer, bmpOut, MARLIN_BMP_SIZE);
#else
	size_t rd;
	if ((rd = ::read(spiFileDesc, spiBuffer, MARLIN_BMP_SIZE)) != MARLIN_BMP_SIZE)
	{
		::close(spiFileDesc);
		spiError = true;

		return true;
	}
#endif // __TX_RAW

	spiError = false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// close SPI1

void CMarlinBridge::close()
{
	if (settings[DEMO_MODE].val || spiError)
		return;
	
	if (::close(spiFileDesc) < 0)
	{
		spiError = true;
		
		return;
	}

	spiError = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// animate two demo Marlin UI screen captures

void CMarlinBridge::swapDemoBmp()
{
	static bool b = true;

	memcpy(spiBuffer, (b = !b) ? bmpDemo0 : bmpDemo1, MARLIN_BMP_SIZE);
}

#ifdef __TX_RAW

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// function : converts the OLED bitmap into ordinary bitmap
//
//    SSD1306/SSD1309/SSH1106 : 8 pages with 128 8bit "vertical macropixels"
//    bmpOut : 1024 bytes, 1-bit monochrome linear bitmap (128 lines 64 columns)
//    (input = control bytes are striped by the BluePill (30 microseconds)
//    Similar to a matrix transposition, each elementaty matrix being 8x8
//    duration : 2.80ms on BluePill

// data (SSD1306/SSD1309) : 1 frame = 8 pages, 1 page = 3 command bytes + 128 bytes, 1 page = 8 graphic lines
const int OLED_PAGE_COUNT		= 8;
const int OLED_LINES_PER_PAGE	= 8;
const int OLED_PAGE_SIZE		= 128;

void bmpOledToLinear(uint8_t* bmpOut, const uint8_t** bmpIn)
{
	//uint32 t0 = micros();

	uint16_t iByte = 0; // bytes counter for output bitmap
	uint16_t jBit = 0; // bits counter for the current byte
	memset((void*)bmpOut, 0, OLED_PAGE_COUNT * OLED_PAGE_SIZE); // reset to zero

	for (uint8_t page = 0; page < OLED_PAGE_COUNT; page++)
		for (uint8_t line = 0; line < OLED_LINES_PER_PAGE; line++)
			for (uint8_t col = 0; col < OLED_PAGE_SIZE; col++)
			{
				// vertical byte to horizontal byte
				if (bmpIn[page][col] & (1 << line))
					bmpOut[iByte] |= (uint8_t)(0x01 << (7 - jBit));

				jBit++;
				jBit %= 8;

				if (jBit == 0) // byte complete
					iByte++; // next byte
			}

	//Serial.println(micros() - t0);
}

#endif // __TX_RAW
