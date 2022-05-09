/*

	Settings.h

*/

#pragma once

#include <stdint.h>
#include <string>
#include <map>

#include <wiringPi.h>

// config.yaml
const char CONFIG_YAML[]	= "/home/pi/.octoprint/config.yaml";
const char PLUGIN_SECTION[]	= "marlinmode:";

// some predefined colors
const uint32_t SOLID_BLACK = 0xff000000;

// STM32 bridge dependant ; HARDCODED
// LCD bitmap (1 bit per pixel, big endian, 128x64 pixels = 1024 bytes)
const uint32_t LCD_WIDTH		= 128;
const uint32_t LCD_HEIGHT		= 64;
const uint32_t MARLIN_BMP_SIZE	= (LCD_WIDTH * LCD_HEIGHT) / 8; // Marlin UI size in bytes 
const uint32_t DEFAULT_MARGIN	= 2; // default margin to added around the 128x64 MarlinUI bitmap (pixels)

extern uint32_t fgndColor;		// current foreground color
extern uint32_t bkgndColor;		// current background color

// predefined quaLity
typedef enum
{
	QLTY_LOW,
	QLTY_MEDIUM,
	QLTY_HIGH,
} GFX_QUALITY_t;

// predefined alignments
typedef enum
{
	POS_TOP_LEFT,		POS_TOP_CENTER,		POS_TOP_RIGHT,
	POS_CENTER_LEFT,	POS_CENTER_CENTER,	POS_CENTER_RIGHT,
	POS_BOTTOM_LEFT,	POS_BOTTOM_CENTER,	POS_BOTTOM_RIGHT
} DISPLAY_POS_t;

// predefined sizings
typedef enum
{
	DISPLAY_MODE_MARLIN,
	DISPLAY_MODE_CUSTOM
} DISPLAY_MODE_t;

// themes
typedef enum
{
	THEME_LCD_CLASSIC = 0,
	THEME_LCD_LBLUE_BLUE,
	THEME_LCD_BLACK_WHITE,
	THEME_LCD_BLACK_YELLOW,
	THEME_LCD_BLACK_BLUE,
	THEME_LCD_BLACK_GREEN,
	THEME_HEADACHE,
	THEME_OLED_BLUE,
	THEME_OLED_WHITE,
	THEME_OLED_YELLOW,
	THEME_OLED_GREEN,
	THEME_RETINA_DAMAGE
} DISPLAY_THEME_t;

// default settings
// Appearance
const bool				DEFAULT_DEMO_MODE			= true;
const GFX_QUALITY_t		DEFAULT_GFX_QUALITY			= QLTY_LOW;
const DISPLAY_MODE_t	DEFAULT_DISPLAY_MODE		= DISPLAY_MODE_CUSTOM;
const DISPLAY_THEME_t	DEFAULT_MARLIN_MODE_THEME	= THEME_LCD_CLASSIC;
const uint32_t			DEFAULT_SIZE				= 50;
const DISPLAY_POS_t		DEFAULT_POSITION			= POS_CENTER_CENTER;
const uint32_t			DEFAULT_FGND_COLOR			= 0x00FFFFFF;
const uint32_t			DEFAULT_FGND_OPACITY		= 50;
const uint32_t			DEFAULT_BKGND_COLOR			= 0x000000FF;
const uint32_t			DEFAULT_BKGND_OPACITY		= 50;
// System
const bool				DEFAULT_SHOW_AT_STARTUP		= true;
const bool				DEFAULT_DISABLE_HID			= false;
const uint32_t			DEFAULT_SPI_SPEED			= 16;	// MHz
const uint32_t			DEFAULT_DTR_TIMEOUT			= 3000;
const uint32_t			DEFAULT_MARLIN_BTN_DEBOUNCE = 300;
// Hardware
const uint32_t			DEFAULT_SPI_DEV				= 2;
const uint32_t			DEFAULT_DTR_PIN				= 5;
const uint32_t			DEFAULT_DTR_PUD				= PUD_OFF;
const uint32_t			DEFAULT_DTR_EDGE			= INT_EDGE_RISING;
const uint32_t			DEFAULT_MARLIN_BTN_PIN		= 4;	// GPIO4, pin 7 ; pullup
const uint32_t			DEFAULT_MARLIN_BTN_PUD		= PUD_UP;
const uint32_t			DEFAULT_MARLIN_BTN_EDGE		= INT_EDGE_RISING;
const uint32_t			DEFAULT_ENCODER_EN_PIN		= 6;	// GPIO6, pin 31
const uint32_t			DEFAULT_ENCODER_EN_ACTIVE	= 1;

// dictiionary index
typedef enum
{
	// Appearence
	DEMO_MODE,
	GFX_QUALITY,
	DISPLAY_MODE,
	MARLIN_MODE_THEME,
	SIZE,
	POSITION,
	FGND_COLOR,
	FGND_OPACITY,
	BKGND_COLOR,
	BKGND_OPACITY,
	// System
	SHOW_AT_STARTUP,
	DISABLE_HID,
	SPI_SPEED,
	DTR_TIMEOUT,
	MARLIN_BTN_DEBOUNCE,
	// Hardware
	SPI_DEV,
	DTR_PIN,
	DTR_PUD,
	DTR_EDGE,
	MARLIN_BTN_PIN,
	MARLIN_BTN_PUD,
	MARLIN_BTN_EDGE,
	ENCODER_EN_PIN,
	ENCODER_EN_ACTIVE,
} SETTING_ID_t;

// quick and dirty dictionary
typedef struct
{
	SETTING_ID_t	id;
	std::string		key;
	uint32_t		val;
	uint32_t		defVal;
}  entry;

extern entry settings[];

// quick and dirty dictionary
typedef struct
{
	DISPLAY_THEME_t	id;
	uint32_t		fgndColor;
	uint32_t		bkgndColor;
}  theme;

class themeDict // static class
{
public:

	static theme themes[];
};

bool getSettingsFromConfigYaml();
void initIO();				// in main.cpp
void initIO();				// definition in main.cpp
const char* getSpiDev();	// device name string