
/*
	MarlinOSD / marlin_osd.svc

*/

#include <wiringPi.h>	// linker dependency : wiringPi ; linker option : -lbcm_host
#include <pthread.h>	// linker option : -pthread
#include <mutex>
#include <signal.h>
#include <sys/time.h>

#include "MarlinBridge.h"
#include "Errors.h"
#include "EventGrabber.h"
#include "MarlinWnd.h"

// should be commented out for debug only
#define __RUN_AS_A_SERVICE

// Marlin OSD button ISR/thread
void MarlinModeBtnISR();
pthread_cond_t pthread_cond_MarlinMode = PTHREAD_COND_INITIALIZER;
pthread_mutex_t pthread_mtx_MarlinMode = PTHREAD_MUTEX_INITIALIZER;

// bitmap ready ISR/thread
void bmpReadyISR();
pthread_cond_t pthread_cond_bmpReady = PTHREAD_COND_INITIALIZER;
pthread_mutex_t pthread_mtx_bmpReady = PTHREAD_MUTEX_INITIALIZER;

void onTerminate(int);

volatile bool showMarlin = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main function

int main(/*int argc, char** argv*/)
{
	// for cleanup on exit
	signal(SIGTERM, onTerminate);	// terminate
	signal(SIGINT, onTerminate);	// Ctrl+C
	signal(SIGHUP, onTerminate);	// hang up
	signal(SIGKILL, onTerminate);	// usefull ???
	signal(SIGABRT, onTerminate);	// usefull ???

	// if fails, will display demo (fake Marlin UI)
#ifdef	__RUN_AS_A_SERVICE
	system("dtoverlay spi1-1cs cs0_pin=16");
#endif
	// initialize GPU
	bcm_host_init();

	// setup GPIO
	wiringPiSetupGpio(); // always returns 0 : http://wiringpi.com/reference/setup/
	pinMode(PIN_DATA_READY, INPUT); // from STM32, no pullup needed
	pinMode(PIN_MARLIN_MODE_BTN, INPUT);
	pullUpDnControl(PIN_MARLIN_MODE_BTN, PUD_UP); // needed if no hat present (-> demo mode)
	pinMode(PIN_ENC_EN, OUTPUT);

	// set ISRs
	if (wiringPiISR(PIN_DATA_READY, INT_EDGE_RISING, bmpReadyISR) < 0)
		exitError(ERROR_BTN_ISR, __FILE__, __LINE__);

	if (wiringPiISR(PIN_MARLIN_MODE_BTN, INT_EDGE_RISING, MarlinModeBtnISR) < 0)
		exitError(ERROR_BTN_ISR, __FILE__, __LINE__);

	// Marlin encoder enable pin
	getSettingsFromConfigYaml();
	showMarlin = settings[SHOW_AT_BOOT].val;
	digitalWrite(PIN_ENC_EN, showMarlin); // encoder : enable = HIGH, disable = LOW

	// main loop
	while (~0)
	{
		if (!showMarlin)
		{
			// disable Marlin encoder
			digitalWrite(PIN_ENC_EN, FALSE); // enable = HIGH, disable = LOW

			// wait for Marlin button
			if(pthread_mutex_lock(&pthread_mtx_MarlinMode) != 0)
				exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

			if(pthread_cond_wait(&pthread_cond_MarlinMode, &pthread_mtx_MarlinMode) != 0)
				exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

			if(pthread_mutex_unlock(&pthread_mtx_MarlinMode))
				exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);
		}
		
		// show Marlin : initialize OSD
		digitalWrite(PIN_ENC_EN, TRUE); // enable rotary encoder
		CMarlinBridge::open();
		CMarlinWnd::init();

		while (showMarlin) // showMarlin is updated by MarlinModeBtnISR()
		{
			if (settings[DEMO_MODE].val || CMarlinBridge::spiError) // if SPI does not respond : set by CMarlinBridge::open()
			{
				digitalWrite(PIN_ENC_EN, FALSE); // disable rotary encoder
				// alternate between 2 demo frames
				delay(830);
				CMarlinBridge::swapDemoBmp();
			}
			else
			{
				// wait for data ready pulse
				if (pthread_mutex_lock(&pthread_mtx_bmpReady) != 0)
					exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

				if (pthread_cond_wait(&pthread_cond_bmpReady, &pthread_mtx_bmpReady) != 0)
					exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

				if (pthread_mutex_unlock(&pthread_mtx_bmpReady) != 0)
					exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

				// data are ready...
				CMarlinBridge::read();
			}

			// paint first, or display will be laggy
			CMarlinWnd::paint();

			getSettingsFromConfigYaml();

			if (settings[DISABLE_HID].val == true || settings[DISPLAY_MODE].val == DISPLAY_MODE_MARLIN)
				CEventGrabber::disableHIDs();
			else
				CEventGrabber::enableHIDs();
		}

		// end show Marlin : terminate OSD
		digitalWrite(PIN_ENC_EN, FALSE);  // disable Marlin rotary encoder
		CMarlinWnd::close();
		CMarlinBridge::close();
		CEventGrabber::enableHIDs();
	}

	exit(EXIT_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Marlin OSD button ISR

void MarlinModeBtnISR()
{
	// debouncing
	static uint32_t last_interrupt_time = 0;
	uint32_t interrupt_time = millis(); // wiringPi millis()

	if (interrupt_time - last_interrupt_time < settings[DEBOUNCE_TIME].val)
		return;

	last_interrupt_time = interrupt_time;

	showMarlin = !showMarlin;

	// end "bmp ready" thread if active, so there will be no attempts
	// to display MarlinUI while closing SPI and display
	if (pthread_cond_broadcast(&pthread_cond_bmpReady) != 0)
		exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

	if (pthread_mutex_lock(&pthread_mtx_MarlinMode) != 0)
		exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

	if (pthread_cond_signal(&pthread_cond_MarlinMode) != 0)
		exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

	if(pthread_mutex_unlock(&pthread_mtx_MarlinMode) != 0)
		exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Marlin OSD "bitmap ready" pin ISR

void bmpReadyISR()
{
	if(pthread_mutex_lock(&pthread_mtx_bmpReady) != 0)
		exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

	if (pthread_cond_signal(&pthread_cond_bmpReady) != 0)
		exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

	if(pthread_mutex_unlock(&pthread_mtx_bmpReady) != 0)
		exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// termination

void onTerminate(int)
{
	// cleanup
	if (showMarlin)
	{
		CMarlinWnd::close();
		CMarlinBridge::close();
	}

	CEventGrabber::enableHIDs();
	exit(EXIT_SUCCESS);
}
