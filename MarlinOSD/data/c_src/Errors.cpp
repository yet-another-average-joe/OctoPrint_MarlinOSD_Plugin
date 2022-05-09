/*

	Errors.cpp

*/


#include "Errors.h"

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <libgen.h>


void exitError(int err, const char* file, int line)
{
	const char* strErr;

	switch (err)
	{
	case ERROR_TODO:
		strErr = "Fatal Error : TODO";
		break;
	case ERROR_WIRING_PI_SETUP:
		strErr = "Fatal Error : Unable to setup wiringPi.";
		break;

		// SPI

	case ERROR_SPI_OPEN:
		strErr = "Fatal Error : SPI open.";
		break;
	case ERROR_SPI_IOCTL:
		strErr = "Fatal Error : SPI ioctl.";
		break;
	case ERROR_SPI_READ:
		strErr = "Fatal Error : SPI read.";
		break;
	case ERROR_SPI_CLOSE:
		strErr = "Fatal Error : SPI close.";
		break;

		// Button ISR

	case ERROR_BTN_ISR:
		strErr = "Fatal Error : Marlin OSD button ISR";
		break;
	//case ERROR_BTN_THRD_COND_SIG:
	//	strErr = "Fatal Error : MarlinMode button ISR thread signal condition.";
	//	break;
	
	//case ERROR_BTN_THRD_COND_BROADCAST:
	//	strErr = "Fatal Error : MarlinMode button ISR thread condition broadcast.";
	//	break;
	
	//case ERROR_BTN_THRD_COND_WAIT:
	//	strErr = "Fatal Error : MarlinMode button ISR thread wait condition.";
	//	break;
	
	//case ERROR_BTN_SHUTDOWN:
	//	strErr = "Fatal Error : Shutdown button ISR";
	//	break;

		// Bitmap Ready

	case ERROR_DATA_READY_ISR:
		strErr = "Fatal Error : Data Ready ISR.";
		break;
	
		//case ERROR_BMP_READY_THRD_COND_SIG:
	//	strErr = "Fatal Error : Bitmap Ready ISR thread signal condition.";
	//	break;
	
	//case ERROR_BMP_READY_THRD_COND_WAIT:
	//	strErr = "Fatal Error : Bitmap Ready ISR thread wait condition.";
	//	break;

	// event manager

	//case ERROR_NO_EVENT_DEVICES:
	//	strErr = "Fatal Error : no event devices.";
	//	break;

	case ERROR_FAILED_OPEN_EVENT_DEVICE:
		strErr = "Fatal Error : opening event device.";
		break;

	case ERROR_FAILED_GRAB_EVENTS:
		strErr = "Fatal Error : grabbing device events.";
		break;

	// dispmanx
	case ERROR_DISPMANX:
		strErr = "Fatal Error : dispmanx.";
		break;

	case ERROR_FAILED_UNGRAB_EVENTS:
		strErr = "Fatal Error : failed ungrab events.";
		break;

	case ERROR_FAILED_CLOSE_EVENT_DEVICE:
		strErr = "Fatal Error : failed close event device.";
		break;

	case ERROR_MULTITHREADING:
		strErr = "Fatal Error : mutex.";
		break;

	case ERROR_FAILED_OPEN_CONFIG_YAML:
		strErr = "Fatal Error : failed open config.yaml.";
		break;

	default: // should not happen !
		strErr = "Fatal Error : UNKNOWN.";
		break;
	}

	//CMarlinWnd::close();
	//CMarlinBridge::close();
	//CEventMngr::disableHID(false);

	char buffer[256];
	sprintf(buffer, "%s : %s, line %d\n", strErr, basename((char*)file), line);
	perror(buffer);
	printf(buffer);

	exit(err);
}

