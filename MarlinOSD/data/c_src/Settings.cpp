/*

	Settings.cpp

*/

#include "Settings.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "Errors.h"
#include "MarlinWnd.h"
#include "MarlinBridge.h"
#include "Util.h"

#include <wiringPi.h>	// linker dependency : wiringPi ; linker option : -lbcm_host

//using namespace std; // problem with remove_if()

// default values : have to be defined in __init__.py

// dictionary
entry settings[] =
{
	//		ID				config.yaml key				current value					default value
	{ DEMO_MODE,			"demo_mode",			DEFAULT_DEMO_MODE,				DEFAULT_DEMO_MODE },
	{ GFX_QUALITY,			"gfx_quality",			DEFAULT_GFX_QUALITY,			DEFAULT_GFX_QUALITY },
	{ DISPLAY_MODE,			"display_mode",			DEFAULT_DISPLAY_MODE,			DEFAULT_DISPLAY_MODE },
	{ MARLIN_MODE_THEME,	"marlin_mode_theme",	DEFAULT_MARLIN_MODE_THEME,		DEFAULT_MARLIN_MODE_THEME },
	{ SIZE,					"size",					DEFAULT_SIZE,					DEFAULT_SIZE },
	{ POSITION,				"position",				DEFAULT_POSITION,				DEFAULT_POSITION },
	{ FGND_COLOR,			"fgnd_color",			DEFAULT_FGND_COLOR,				DEFAULT_FGND_COLOR},
	{ FGND_OPACITY,			"fgnd_opacity",			DEFAULT_FGND_OPACITY,			DEFAULT_FGND_OPACITY },
	{ BKGND_COLOR,			"bkgnd_color",			DEFAULT_BKGND_COLOR,			DEFAULT_BKGND_COLOR },
	{ BKGND_OPACITY,		"bkgnd_opacity",		DEFAULT_BKGND_OPACITY,			DEFAULT_BKGND_OPACITY },

	{ SHOW_AT_STARTUP,		"show_at_startup",		DEFAULT_SHOW_AT_STARTUP,		DEFAULT_SHOW_AT_STARTUP },
	{ DISABLE_HID,			"disable_HID",			DEFAULT_DISABLE_HID,			DEFAULT_DISABLE_HID },
	{ SPI_SPEED,			"spi_speed",			DEFAULT_SPI_SPEED,				DEFAULT_SPI_SPEED },
	{ DTR_TIMEOUT,			"dtr_timeout",			DEFAULT_DTR_TIMEOUT,			DEFAULT_DTR_TIMEOUT, },
	{ MARLIN_BTN_DEBOUNCE,	"marlin_btn_debounce",	DEFAULT_MARLIN_BTN_DEBOUNCE,	DEFAULT_MARLIN_BTN_DEBOUNCE },

	{ SPI_DEV,				"spi_dev",				DEFAULT_SPI_DEV,				DEFAULT_SPI_DEV },
	{ DTR_PIN,				"dtr_pin",				DEFAULT_DTR_PIN,				DEFAULT_DTR_PIN },
	{ DTR_PUD,				"dtr_pud",				DEFAULT_DTR_PUD,				DEFAULT_DTR_PUD },
	{ DTR_EDGE,				"dtr_edge",				DEFAULT_DTR_EDGE,				DEFAULT_DTR_EDGE },
	{ MARLIN_BTN_PIN,	  	"marlin_btn_pin",		DEFAULT_MARLIN_BTN_PIN, 		DEFAULT_MARLIN_BTN_PIN, },
	{ MARLIN_BTN_PUD,	  	"marlin_btn_pud",		DEFAULT_MARLIN_BTN_PUD, 		DEFAULT_MARLIN_BTN_PUD, },
	{ MARLIN_BTN_EDGE,	  	"marlin_btn_edge",		DEFAULT_MARLIN_BTN_EDGE, 		DEFAULT_MARLIN_BTN_EDGE, },
	{ ENCODER_EN_PIN,		"encoder_en_pin",		DEFAULT_ENCODER_EN_PIN,			DEFAULT_ENCODER_EN_PIN },
	{ ENCODER_EN_ACTIVE,	"encoder_en_active",	DEFAULT_ENCODER_EN_ACTIVE,		DEFAULT_ENCODER_EN_ACTIVE },
};

const uint32_t SETTINGS_SIZE = sizeof(settings) / sizeof(entry);

theme themeDict::themes[] =
{
	{ THEME_LCD_CLASSIC,			0xffffffff, 0xff0000ff},
	{ THEME_LCD_LBLUE_BLUE,			0xff0fE0ff, 0xff0000ff},
	{ THEME_LCD_BLACK_WHITE,		0xff404040, 0xffe5e5e5},
	{ THEME_LCD_BLACK_YELLOW,		0xff404040, 0xffeefd86},
	{ THEME_LCD_BLACK_BLUE,			0xff404040, 0xff8a8aff},
	{ THEME_LCD_BLACK_GREEN,		0xff404040, 0xff55ff55},
	{ THEME_HEADACHE,				0xffff0000, 0xff0000ff},
	{ THEME_OLED_BLUE,				0xff27f3ee, 0xff000000},
	{ THEME_OLED_WHITE,				0xffffffff, 0xff000000},
	{ THEME_OLED_YELLOW,			0xffffff00, 0xff000000},
	{ THEME_OLED_GREEN,				0xff32ee2d, 0xff000000},
	{ THEME_RETINA_DAMAGE,			0xff0000ff, 0xffff0000}
};

const uint32_t THEMES_SIZE = sizeof(themeDict::themes) / sizeof(theme);

static const char* spiPort[] =
{
	"/dev/spidev0.0",
	"/dev/spidev0.1",
	"/dev/spidev1.0",
	"/dev/spidev1.1",
	"/dev/spidev1.2",
};

uint32_t fgndColor;		// current foreground color
uint32_t bkgndColor;	// current background color

// helpers
static inline bool not_isalnum(char c) { return !isalnum(c); }
static void parseSettings(std::ifstream& strm);
static void updateColors();

const char* getSpiDev()
{
	return spiPort[settings[SPI_DEV].val];
}

static inline bool seekSection(std::ifstream& strm, std::string str)
{
	std::string line;

	while (getline(strm, line))
	{
		if (line[0] == ' ') // not a section...
			continue;

		if (line.find(str)) // section
			return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// updates the window attributes and system options from config.yaml
// < 1 millisecond as long as config.yaml is cached
// -> no need for optimisation

bool getSettingsFromConfigYaml()
{
	std::ifstream cFile(CONFIG_YAML);

	if (cFile.is_open())
	{
		seekSection(cFile, "plugins:");	// plugins
		seekSection(cFile, PLUGIN_SECTION);
		parseSettings(cFile);
		cFile.close();
	}
	else
	{
		exitError(ERROR_FAILED_OPEN_CONFIG_YAML, __FILE__, __LINE__);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// parses a value in config.yaml

static void parseSettings(std::ifstream& strm)
{
	std::string line;

	// viewport reinit required
	static unsigned gfx_qlty_old			= settings[GFX_QUALITY].val;
	static unsigned display_mode_old		= settings[DISPLAY_MODE].val;
	static unsigned theme_old				= settings[MARLIN_MODE_THEME].val;
	static unsigned size_old				= settings[SIZE].val;
	static unsigned position_old			= settings[POSITION].val;
	static unsigned spi_speed_old			= settings[SPI_SPEED].val;
	static unsigned dtr_timeout_old			= settings[DTR_TIMEOUT].val;
	static unsigned marlin_btn_debounce_old = settings[MARLIN_BTN_DEBOUNCE].val;

	// wiringPi reboot required
	static unsigned dtr_pin_old				= settings[DTR_PIN].val;
	static unsigned dtr_pud_old				= settings[DTR_PUD].val;
	static unsigned dtr_edge_old			= settings[DTR_EDGE].val;
	static unsigned marlin_btn_pin_old		= settings[MARLIN_BTN_PIN].val;
	static unsigned marlin_btn_pud_old		= settings[MARLIN_BTN_PUD].val;
	static unsigned marlin_btn_edge_old		= settings[MARLIN_BTN_EDGE].val;
	static unsigned encoder_en_pin_old		= settings[ENCODER_EN_PIN].val;
	static unsigned encoder_en_active_old	= settings[ENCODER_EN_ACTIVE].val;

	// reset all settings to defaults : OctoPrint does not persist settings that don't differ from defaults
	// will reset parameters in case an error occurs while retrieving data

	for (unsigned i = 0; i < SETTINGS_SIZE; i++)
		settings[i].val = settings[i].defVal;

	while (getline(strm, line))
	{
		line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

		if (line[0] == '#' || line.empty()) // comment or empty line
			continue;

		std::size_t delimiterPos = line.find(":");
		std::string key = line.substr(0, delimiterPos);
		std::string strVal = line.substr(delimiterPos + 1);

		// matching key ?
		for (unsigned i = 0; i < SETTINGS_SIZE; i++)
		{
			if (key == settings[i].key)
			{
				switch (settings[i].id)
				{
					// boolean
				case SHOW_AT_STARTUP:
				case DISABLE_HID:
				case DEMO_MODE:
					if (strVal == "false")
						settings[i].val = false;
					else if (strVal == "true")
						settings[i].val = true;
					else if (strVal == "0")
						settings[i].val = false;
					else if (strVal == "1")
						settings[i].val = true;
					break;

				case FGND_COLOR:
				case BKGND_COLOR:
					strVal.erase(std::remove_if(strVal.begin(), strVal.end(), not_isalnum), strVal.end());
					settings[i].val = (unsigned)std::stoi(strVal.c_str(), nullptr, 16);
					break;

				// number
				case GFX_QUALITY:
				case DISPLAY_MODE:
				case MARLIN_MODE_THEME:
				case SIZE:
				case POSITION:
				case FGND_OPACITY:
				case BKGND_OPACITY:
				case SPI_SPEED:
				case DTR_TIMEOUT:
				case MARLIN_BTN_DEBOUNCE:
				case SPI_DEV:
				case DTR_PIN:
				case DTR_PUD:
				case DTR_EDGE:
				case MARLIN_BTN_PIN:
				case MARLIN_BTN_PUD:
				case MARLIN_BTN_EDGE:
				case ENCODER_EN_PIN:
				case ENCODER_EN_ACTIVE:
					strVal.erase(std::remove_if(strVal.begin(), strVal.end(), not_isalnum), strVal.end());
					settings[i].val = (unsigned)atoi(strVal.c_str());
					// TODO: sanity check
					break;
					
				default: // unknown : should not happen...
					// TODO : ERROR
					break;
				}

				break;
			}
		}
	}

	// if window settings changed
	if (gfx_qlty_old		!= settings[GFX_QUALITY].val		||
		display_mode_old	!= settings[DISPLAY_MODE].val		||
		theme_old			!= settings[MARLIN_MODE_THEME].val	||
		size_old			!= settings[SIZE].val				||
		position_old		!= settings[POSITION].val)
	{
		if (CMarlinWnd::isOpen())
		{
			// reinit window
			CMarlinWnd::close();
			CMarlinWnd::init();
			CMarlinWnd::paint();
		}
	}

	//  if  changed
	if (spi_speed_old != settings[SPI_SPEED].val)
	{
		// reinit SPI
		CMarlinBridge::close();
		CMarlinBridge::open();
		CMarlinBridge::read();
	}

	gfx_qlty_old			= settings[GFX_QUALITY].val;
	display_mode_old		= settings[DISPLAY_MODE].val;
	theme_old				= settings[MARLIN_MODE_THEME].val;
	size_old				= settings[SIZE].val;
	position_old			= settings[POSITION].val;
	spi_speed_old			= settings[SPI_SPEED].val;
	dtr_timeout_old			= settings[DTR_TIMEOUT].val;
	marlin_btn_debounce_old = settings[MARLIN_BTN_DEBOUNCE].val;

	dtr_pin_old				= settings[DTR_PIN].val;
	dtr_pud_old				= settings[DTR_PUD].val;
	dtr_edge_old			= settings[DTR_EDGE].val;
	marlin_btn_pin_old		= settings[MARLIN_BTN_PIN].val;
	marlin_btn_pud_old		= settings[MARLIN_BTN_PUD].val;
	marlin_btn_edge_old		= settings[MARLIN_BTN_EDGE].val;
	encoder_en_pin_old		= settings[ENCODER_EN_PIN].val;
	encoder_en_active_old	= settings[ENCODER_EN_ACTIVE].val;

	// update colors
	updateColors();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// helper 

void updateColors()
{
	if (settings[DISPLAY_MODE].val == DISPLAY_MODE_MARLIN)
	{
		uint32_t id = settings[MARLIN_MODE_THEME].val;

		for (unsigned i = 0; i < SETTINGS_SIZE; i++)
		{
			if (id == themeDict::themes[i].id)
			{
				fgndColor = themeDict::themes[i].fgndColor;
				bkgndColor = themeDict::themes[i].bkgndColor;

				return;
			}
		}
	}
	else
	{
		fgndColor = settings[FGND_COLOR].val | percentToByte(settings[FGND_OPACITY].val) << 24;
		bkgndColor = settings[BKGND_COLOR].val | percentToByte(settings[BKGND_OPACITY].val) << 24;
	}
}

