
/*

	MarlinOSD/marlin_osd

	
	source ~/oprint/bin/activate
	cd MarlinOSD
	octoprint dev plugin:install

	cd oprint/lib/python3.7/site-packages/MarlinOSD/data/c_src
	make cleanobj

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

// turns a pin HIGH/LOW before/after painting the dispmanx window
// for logic analyser / scope

//#define __PROFILING

#ifdef __PROFILING

//const int testPin =
unsigned int testPulseWidth = 10;
unsigned int paintDuration = 0;

#endif //  __PROFILING

// Marlin OSD button ISR/thread
void MarlinModeBtnISR();
pthread_cond_t pthread_cond_MarlinMode = PTHREAD_COND_INITIALIZER;
pthread_mutex_t pthread_mtx_MarlinMode = PTHREAD_MUTEX_INITIALIZER;

// bitmap ready ISR/thread
void bmpReadyISR();
pthread_cond_t pthread_cond_bmpReady = PTHREAD_COND_INITIALIZER;
pthread_mutex_t pthread_mtx_bmpReady = PTHREAD_MUTEX_INITIALIZER;

void onTerminate(int);

volatile bool showMarlin = false; // updated by MarlinModeBtnISR()

// Helpers

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper : enable / disable rotary encoder

inline void enableEncoder(bool en = TRUE)
{
	digitalWrite(settings[ENCODER_EN_PIN].val, settings[ENCODER_EN_ACTIVE].val ? en : !en);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper : enable / disable HID depnding on HID flag and didplay mode

inline void updateHIDsStatus()
{
	if (settings[DISABLE_HID].val == true || settings[DISPLAY_MODE].val == DISPLAY_MODE_MARLIN)
		CEventGrabber::disableHIDs();
	else
		CEventGrabber::enableHIDs();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper : update display

inline void refreshDisplay()
{
#ifdef __PROFILING
	//digitalWrite(testPin, HIGH);
	unsigned int t0 = micros();
#endif //  __PROFILING

	getSettingsFromConfigYaml();
	updateHIDsStatus();
	CMarlinBridge::read();
	CMarlinWnd::paint();

#ifdef __PROFILING
	paintDuration = micros() - t0;
	//digitalWrite(testPin, LOW);
#endif //  __PROFILING
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper : GPIO initializations

void initIO()
{
	// setup GPIO
	wiringPiSetupGpio(); // always returns 0 : http://wiringpi.com/reference/setup/

	// DTR
	pinMode(settings[DTR_PIN].val, INPUT);
	pullUpDnControl(settings[DTR_PIN].val, settings[DTR_PUD].val);

	if (wiringPiISR(settings[DTR_PIN].val, settings[DTR_EDGE].val, bmpReadyISR) < 0)
		exitError(ERROR_BTN_ISR, __FILE__, __LINE__);

	// Marlin button
	pinMode(settings[MARLIN_BTN_PIN].val, INPUT);
	pullUpDnControl(settings[MARLIN_BTN_PIN].val, settings[MARLIN_BTN_PUD].val);

	if (wiringPiISR(settings[MARLIN_BTN_PIN].val, settings[MARLIN_BTN_EDGE].val, MarlinModeBtnISR) < 0)
		exitError(ERROR_BTN_ISR, __FILE__, __LINE__);

	// encoder enable
	pinMode(settings[ENCODER_EN_PIN].val, OUTPUT);

	// set ISRs
	if (wiringPiISR(settings[MARLIN_BTN_PIN].val, INT_EDGE_RISING, MarlinModeBtnISR) < 0)
		exitError(ERROR_BTN_ISR, __FILE__, __LINE__);

#ifdef __PROFILING
	//pinMode(testPin, OUTPUT);
	//digitalWrite(testPin, LOW);
#endif //  __PROFILING

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main function

int main(/*int argc, char** argv*/)
{
	signal(SIGTERM, onTerminate);	// terminate
	signal(SIGINT, onTerminate);	// Ctrl+C

	// usefull ???
	signal(SIGHUP, onTerminate);
	signal(SIGKILL, onTerminate);
	signal(SIGABRT, onTerminate);

	bcm_host_init(); // initialize GPU

	getSettingsFromConfigYaml();	// get custom IO settings if any
	initIO();						// IO initialization
	showMarlin = settings[SHOW_AT_STARTUP].val;
	enableEncoder(showMarlin); // encoder : enable = HIGH, disable = LOW
	updateHIDsStatus();
	
	while (~0) // main loop
	{
		if (!showMarlin)
		{
			// disable Marlin encoder if Marlin UI is not visible
			enableEncoder(FALSE);

			// wait for Marlin button
			if(pthread_mutex_lock(&pthread_mtx_MarlinMode) != 0)
				exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

			if(pthread_cond_wait(&pthread_cond_MarlinMode, &pthread_mtx_MarlinMode) != 0)
				exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

			if(pthread_mutex_unlock(&pthread_mtx_MarlinMode))
				exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);
		}
		
		// Marlin button was pressed
		CMarlinBridge::open();
		CMarlinWnd::init();
		refreshDisplay(); // display now, don't wait for DTR

		while (showMarlin) // updated by MarlinModeBtnISR()
		{
			// if demo mode or SPI does not respond (set by CMarlinBridge::open()) -> display demo
			if (settings[DEMO_MODE].val || CMarlinBridge::spiError)
			{
				// allways disable rotary encoder while in demo mode
				enableEncoder(FALSE);

				// alternate between 2 demo frames
				delay(1000);
				CMarlinBridge::swapDemoBmp();
			}
			else
			{
				enableEncoder(TRUE);

				// wait for data ready (DTR) pulse
				if (pthread_mutex_lock(&pthread_mtx_bmpReady) != 0)
					exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

				// set timeout
				timeval tv;
				timespec ts;

				int spi_timeout_ms = settings[DTR_TIMEOUT].val * 1000;

				gettimeofday(&tv, NULL);
				ts.tv_sec = time(NULL) + spi_timeout_ms;
				ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (spi_timeout_ms % 1000);
				ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
				ts.tv_nsec %= (1000 * 1000 * 1000);

				// wait DTR pulse with timeout
				int n = pthread_cond_timedwait(&pthread_cond_bmpReady, &pthread_mtx_bmpReady, &ts);

				if (n == 0)
					CMarlinBridge::spiError = FALSE;
				else if (n == ETIMEDOUT)
					CMarlinBridge::spiError = TRUE; // -> display demo  ; reset by next bmpReadyISR() interrupt

				if (pthread_mutex_unlock(&pthread_mtx_bmpReady) != 0)
					exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);
			}
			
			refreshDisplay(); // got something to display
		}

		// end show Marlin : terminate OSD
		enableEncoder(FALSE);

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

	if (interrupt_time - last_interrupt_time < settings[MARLIN_BTN_DEBOUNCE].val)
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
// Data Ready pin ISR

void bmpReadyISR()
{
	if(pthread_mutex_lock(&pthread_mtx_bmpReady) != 0)
		exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

	if (pthread_cond_signal(&pthread_cond_bmpReady) != 0)
		exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

	if(pthread_mutex_unlock(&pthread_mtx_bmpReady) != 0)
		exitError(ERROR_MULTITHREADING, __FILE__, __LINE__);

	CMarlinBridge::spiError = FALSE;
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
