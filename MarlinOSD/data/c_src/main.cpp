
/*

	MarlinOSD/marlin_osd


	----------------------------------------------------------
	measuring : logic analyzer

	start paint OLED -> DTR rising : 15 ms
	STM32 lag = 15 ms = 1/67 sec

			ROUGHLY :

	OLED data = 1048 bytes @ 1MHz		:  11 ms
		STM32 hat lag					: < 5 ms (= total "propagation time", inc 2.8 ms for tranposition)
		getSettingsFromConfigYaml()		:   1 ms
		read()	@8-16MHz				:   1 ms
		paint()							:   5 / 20 / 40 ms depending on oversampling (LOW / MEDIUM / HIGH)

	lag = end receive by hat -> end painting RasPi VRAM = DTR rising -> end paint()
	@8-16 MHz : 7 / 22 / 40 ms depending on oversampling

	start painting OLED -> end painting video buffer : +15 ms
	(first bit at hat "input" -> last bit written in VRAM)

	@8-16 MHz : 22 / 37 / 55 ms depending on oversampling ; does not include the HDMI + monitor lag !

	--> ~2 ms could be saved transposing the pixel matrix on the RasPi
	= 10% at best @16MHz and low oversampling 

	----------------------------------------------------------

	source ~/oprint/bin/activate
	cd MarlinOSD
	octoprint dev plugin:install

	cd oprint/lib/python3.7/site-packages/MarlinOSD/data/c_src
	make cleanobj

	TODO : sanity check size !!! <= 100%

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


//#define __MEASURE // for logic analyser / scope

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper : enable / disable rotary encoder

inline void enableEncoder(bool en = TRUE)
{
	digitalWrite(settings[ENCODER_EN_PIN].val, settings[ENCODER_EN_ACTIVE].val ? en : !en);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper : enable / disable HID depending on HID flag and didplay mode

inline void updateHIDsStatus()
{
	if (settings[DISABLE_HID].val == true || settings[DISPLAY_MODE].val == DISPLAY_MODE_MARLIN)
		CEventGrabber::disableHIDs();
	else
		CEventGrabber::enableHIDs();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper : update display

#ifdef __MEASURE

const int testPin = 26;

inline void refreshDisplay()
{
	digitalWrite(testPin, HIGH);

	getSettingsFromConfigYaml();
	// getSettingsFromConfigYaml() :
	// 0.7~1.5 milliseconds

	digitalWrite(testPin, LOW);

	updateHIDsStatus();

	CMarlinBridge::read();
	// read() :
	// 0.7 milliseconds
	// @16 MHz, read() while updating 
	// 24 MHz : 0.6 ms
	// 16 MHZ : 0.7 ms
	// 8 MHz : 1.15 ms
	// 4 MHz : 2.3 ms
	// 2 MHz : 4.6 ms
	// 1 MHz : 10 ms

	digitalWrite(testPin, HIGH);

	CMarlinWnd::paint();
	// paint()
	// LOW : 5ms, MEDIUM : 20 ms, HIGH : 40 ms ; size has no influence
	// DTR -> start Paint() : 1 ms

	digitalWrite(testPin, LOW);
}
#else
inline void refreshDisplay()
{
	getSettingsFromConfigYaml();
	updateHIDsStatus();
	CMarlinBridge::read();
	CMarlinWnd::paint();
}
#endif //  __MEASURE

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

#ifdef __MEASURE
	pinMode(testPin, OUTPUT);
	digitalWrite(testPin, LOW);
#endif //  __MEASURE
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
