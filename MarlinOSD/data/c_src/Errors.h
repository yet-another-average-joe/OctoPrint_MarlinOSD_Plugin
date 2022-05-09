/*

	Errors.h

*/

#pragma once


// error codes
enum errCode
{
	ERROR_TODO=10,
	ERROR_WIRING_PI_SETUP,

	// SPI
	ERROR_SPI_OPEN,		// not used : stitch to demo mode instead
	ERROR_SPI_IOCTL,	// not used : stitch to demo mode instead
	ERROR_SPI_READ,		// not used : stitch to demo mode instead
	ERROR_SPI_CLOSE,	// not used : stitch to demo mode instead

	// Button ISR
	ERROR_BTN_ISR,
	//ERROR_BTN_THRD_COND_SIG,
	//ERROR_BTN_THRD_COND_BROADCAST,
	//ERROR_BTN_THRD_COND_WAIT,
	//ERROR_BTN_SHUTDOWN,

	// Bitmap Ready
	ERROR_DATA_READY_ISR,
	//ERROR_BMP_READY_THRD_COND_SIG,
	//ERROR_BMP_READY_THRD_COND_WAIT,

	// event manager
	//ERROR_NO_EVENT_DEVICES,
	ERROR_FAILED_OPEN_EVENT_DEVICE,
	ERROR_FAILED_GRAB_EVENTS,
	ERROR_FAILED_UNGRAB_EVENTS,
	ERROR_FAILED_CLOSE_EVENT_DEVICE,

	// multithreading
	ERROR_MULTITHREADING,

	// dispmanx
	ERROR_DISPMANX,

	// settings
	ERROR_FAILED_OPEN_CONFIG_YAML,
};

void exitError(int err, const char* file, int line );
