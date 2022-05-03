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

//using namespace std; // problem with remove_if()

// default values : have to be defined in config.yaml
// if don't exist :
// disable Marlin UI button until available

// dictionary
entry settings[] =
{
//		ID				config.yaml name			current value			default value
	{ IMG_QUALITY,		"img_quality",			DEFAULT_IMG_QUALITY,	DEFAULT_IMG_QUALITY },
	{ DISPLAY_MODE,		"display_mode",			DEFAULT_DISPLAY_MODE,	DEFAULT_DISPLAY_MODE },
	{ THEME,			"marlin_mode_theme",	DEFAULT_THEME,			DEFAULT_THEME },
	{ SIZE,				"size",					DEFAULT_SIZE,			DEFAULT_SIZE },
	{ POSITION,			"position",				DEFAULT_POS,			DEFAULT_POS },
	{ FGND_COLOR,		"fgnd_color",			DEFAULT_FGND_COLOR,		DEFAULT_FGND_COLOR},
	{ FGND_OPACITY,		"fgnd_opacity",			DEFAULT_FGND_OPACITY,	DEFAULT_FGND_OPACITY },
	{ BKGND_COLOR,		"bkgnd_color",			DEFAULT_BKGND_COLOR,	DEFAULT_BKGND_COLOR },
	{ BKGND_OPACITY,	"bkgnd_opacity",		DEFAULT_BKGND_OPACITY,	DEFAULT_BKGND_OPACITY },

	{ SPI_SPEED_MHZ,	"spi_speed",			DEFAULT_SPI_SPEED_MHZ,	DEFAULT_SPI_SPEED_MHZ },
	{ DEBOUNCE_TIME,	"debounce_time",		DEFAULT_DEBOUNCE_TIME,	DEFAULT_DEBOUNCE_TIME },
	{ SHOW_AT_BOOT,		"show_at_boot",			DEFAULT_SHOW_AT_BOOT,	DEFAULT_SHOW_AT_BOOT },
	{ DISABLE_HID,		"disable_HID",			DEFAULT_DISABLE_HID,	DEFAULT_DISABLE_HID },
	{ DEMO_MODE,		"demo_mode",			DEFAULT_DEMO_MODE,		DEFAULT_DEMO_MODE },
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

// helpers
static inline bool not_isalnum(char c) { return !isalnum(c); }
static void parseSettings(std::ifstream& strm);

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

bool getSettingsFromConfigYaml()
{
	//std::cout << '\n';

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

	//for(uint i = 0; i < SETTINGS_SIZE; i++)
	//	std::cout << settings[i].key << " : " << settings[i].val << '\n';

	//std::cout << '\n';

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// returns a color by Id

void themeDict::getColors(uint32_t id, uint32_t& fgndColor, uint32_t& bkgndColor)
{
	for (unsigned i = 0; i < SETTINGS_SIZE; i++)
	{
		if (id == themeDict::themes[i].id)
		{
			fgndColor = themeDict::themes[i].fgndColor;
			bkgndColor= themeDict::themes[i].bkgndColor;

			break;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// parses a value in config.yaml

static void parseSettings(std::ifstream& strm)
{
	std::string line;

	static unsigned img_qlty_old		= settings[IMG_QUALITY].val;
	static unsigned display_mode_old	= settings[DISPLAY_MODE].val;
	static unsigned theme_old			= settings[THEME].val;
	static unsigned size_old			= settings[SIZE].val;
	static unsigned position_old		= settings[POSITION].val;
	static unsigned spi_speed_old		= settings[SPI_SPEED_MHZ].val;

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
				case DEMO_MODE:
				case DISABLE_HID:
				case SHOW_AT_BOOT:
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
				case DEBOUNCE_TIME:
				case DISPLAY_MODE:
				case FGND_OPACITY:
				case BKGND_OPACITY:
				case IMG_QUALITY:
				case POSITION:
				case SPI_SPEED_MHZ:
				case SIZE:
				case THEME:
					strVal.erase(std::remove_if(strVal.begin(), strVal.end(), not_isalnum), strVal.end());
					// TODO: sanity check on values
					settings[i].val = (unsigned)atoi(strVal.c_str());
					break;
					
				default: // unknown : should not happen...
					// TODO : ERROR
					break;
				}

				break;
			}
		}
	}

	// if window attributes changed
	if (img_qlty_old		!= settings[IMG_QUALITY].val	||
		display_mode_old	!= settings[DISPLAY_MODE].val	||
		theme_old			!= settings[THEME].val			||
		size_old			!= settings[SIZE].val			||
		position_old		!= settings[POSITION].val		||
		spi_speed_old		!= settings[SPI_SPEED_MHZ].val)
	{
		if (CMarlinWnd::isOpen())
		{
			// reinit window
			CMarlinWnd::close();
			CMarlinWnd::init();
			CMarlinWnd::paint();
		}
	}

	//  if system parameters changed
	if (spi_speed_old != settings[SPI_SPEED_MHZ].val)
	{
		// reinit SPI
		CMarlinBridge::close();
		CMarlinBridge::open();
		CMarlinBridge::read();
	}

	img_qlty_old		= settings[IMG_QUALITY].val;
	display_mode_old	= settings[DISPLAY_MODE].val;
	theme_old			= settings[THEME].val;
	size_old			= settings[SIZE].val;
	position_old		= settings[POSITION].val;
	spi_speed_old		= settings[SPI_SPEED_MHZ].val;
}

