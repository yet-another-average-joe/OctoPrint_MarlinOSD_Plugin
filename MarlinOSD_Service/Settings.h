/*

	Settings.h

*/

#pragma once

#include <stdint.h>
#include <string>

// config.yaml
const char CONFIG_YAML[]	= "/home/pi/.octoprint/config.yaml";
const char PLUGIN_SECTION[]	= "marlinmode:";

bool getSettingsFromConfigYaml();

//extern uint32_t clrPal[16];

inline uint32_t MAKE_COLOR_ARGB8888(uint8_t a, uint8_t r, uint8_t g, uint8_t b) { return ((a << 24) | (r << 16) | (g << 8) | b); }
inline uint32_t MAKE_SOLID_COLOR_ARGB8888(uint8_t r, uint8_t g, uint8_t b) { return ((0xFF << 24) | (r << 16) | (g << 8) | b); }

// some predefined colors
const uint32_t SOLID_BLACK = 0xff000000;

// STM32 bridge dependant ; HARDCODED
// LCD bitmap (1 bit per pixel, big endian, 128x64 pixels = 1024 bytes)
const uint32_t LCD_WIDTH		= 128;
const uint32_t LCD_HEIGHT		= 64;
const uint32_t SPI_FRAME_SIZE	= 1024;

// predefined quaLity
typedef enum
{
	QLTY_LOW,
	QLTY_MEDIUM,
	QLTY_HIGH,
} IMG_QUALITY_t;

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
const IMG_QUALITY_t		DEFAULT_IMG_QUALITY		= QLTY_MEDIUM;
const DISPLAY_MODE_t	DEFAULT_DISPLAY_MODE	= DISPLAY_MODE_CUSTOM;
const DISPLAY_THEME_t	DEFAULT_THEME			= THEME_LCD_CLASSIC;
const DISPLAY_POS_t		DEFAULT_POS				= POS_CENTER_CENTER;
const uint32_t			DEFAULT_SIZE			= 50;
const uint32_t			DEFAULT_FGND_COLOR		= 0x00FFFFFF;
const uint32_t			DEFAULT_FGND_OPACITY	= 100;
const uint32_t			DEFAULT_BKGND_COLOR		= 0x000000FF;
const uint32_t			DEFAULT_BKGND_OPACITY	= 100;
const uint32_t			DEFAULT_MARGIN			= 2;	// default margin to added around the 128x64 MarlinUI bitmap (pixels)
const uint32_t			DEFAULT_SPI_SPEED_MHZ	= 8;	// SPI speed, MHz ; 16 MHz : unstable on breadboard (STM32 crashes)
const uint32_t			DEFAULT_DEBOUNCE_TIME	= 300;
const bool				DEFAULT_SHOW_AT_BOOT	= true;
const bool				DEFAULT_DISABLE_HID		= false;
const bool				DEFAULT_DEMO_MODE		= true;

// GPIO pins
const uint32_t	PIN_DATA_READY		= 5;	// GPIO5, pin 29
const uint32_t	PIN_MARLIN_MODE_BTN = 4;	// GPIO4, pin 7 ; pullup
const uint32_t	PIN_ENC_EN			= 6;	// GPIO6, pin 31

// dictiionary index
typedef enum
{
	IMG_QUALITY,
	DISPLAY_MODE,
	THEME,
	SIZE,
	POSITION,
	FGND_COLOR,
	FGND_OPACITY,
	BKGND_COLOR,
	BKGND_OPACITY,

	SPI_SPEED_MHZ,
	DEBOUNCE_TIME,
	SHOW_AT_BOOT,
	DISABLE_HID,
	DEMO_MODE,

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

	static void getColors(uint32_t id, uint32_t& fgndColor, uint32_t& bkgndColor);
};

