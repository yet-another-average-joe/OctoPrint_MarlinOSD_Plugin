# coding=utf-8

#####################################################################################
#
#
#       Marlin On Screen Display (MarlinOSD) __init__.py
#
#
#####################################################################################

from __future__ import absolute_import
import octoprint.plugin
import os
import subprocess

# image quality (oversampling)
QLTY_LOW=0
QLTY_MEDIUM=1
QLTY_HIGH=2

# mode
DISPLAY_MODE_MARLIN="0"
DISPLAY_MODE_CUSTOM="1"

# Marlin Mode theme
THEME_LCD_CLASSIC       = 0
THEME_LCD_LBLUE_BLUE    = 1
THEME_LCD_BLACK_WHITE   = 2
THEME_LCD_BLACK_YELLOW  = 3
THEME_LCD_BLACK_BLUE    = 4
THEME_LCD_BLACK_GREEN   = 5
THEME_HEADACHE          = 6
THEME_OLED_BLUE         = 7
THEME_OLED_WHITE        = 8
THEME_OLED_YELLOW       = 9
THEME_OLED_GREEN        = 10
THEME_RETINA_DAMAGE     = 11

# position
POS_TOP_LEFT                = 0
POS_TOP_CENTER              = 1
POS_TOP_RIGHT               = 2
POS_CENTER_LEFT             = 3
POS_CENTER_CENTER           = 4
POS_CENTER_RIGHT            = 5
POS_BOTTOM_LEFT             = 6
POS_BOTTOM_CENTER           = 7
POS_BOTTOM_RIGHT            = 8

# pull up/down (from WiringPi)
_PUD_OFF   = 0
_PUD_DOWN  = 1
_PUD_UP    = 2

# edge  (from WiringPi)
_INT_EDGE_SETUP      = 0
_INT_EDGE_FALLING    = 1
_INT_EDGE_RISING     = 2
_INT_EDGE_BOTH       = 3

################################# ALL DEFAULT SETTINGS ###############################

DEFAULT_GFX_QUALITY         = QLTY_LOW
DEFAULT_DISPLAY_MODE        = DISPLAY_MODE_CUSTOM
DEFAULT_MARLIN_MODE_THEME   = THEME_LCD_CLASSIC
DEFAULT_SIZE                = 50
DEFAULT_POSITION            = POS_CENTER_CENTER
DEFAULT_FGND_COLOR          = "#ffffff"    # white
DEFAULT_FGND_OPACITY        = 50
DEFAULT_BKGND_COLOR         = "#0000ff"    # blue
DEFAULT_BKGND_OPACITY       = 50

DEFAULT_SHOW_AT_STARTUP     = True
DEFAULT_DISABLE_HID         = False
DEFAULT_DEMO_MODE           = True
DEFAULT_SPI_SPEED           = 16    # MHz
DEFAULT_DTR_TIMEOUT         = 3000  # ms
DEFAULT_MARLIN_BTN_DEBOUNCE = 300   # ms

DEFAULT_SPI_DEV                 = 2                     # MarlinOctoHat : SPI1-1 = "/dev/spidev1.0"
DEFAULT_DTR_PIN                 = 5                     # MarlinOctoHat : GPIO5, physical pin #29
DEFAULT_DTR_PUD                 = _PUD_OFF
DEFAULT_DTR_EDGE                = _INT_EDGE_RISING
DEFAULT_MARLIN_BTN_PIN          = 4                     # MarlinOctoHat : GPIO4, physical pin #7  pullup
DEFAULT_MARLIN_BTN_PUD          = _PUD_UP
DEFAULT_MARLIN_BTN_EDGE         = _INT_EDGE_FALLING
DEFAULT_ENCODER_EN_PIN          = 6                     # MarlinOctoHat : GPIO6, physical pin #31
DEFAULT_ENCODER_EN_ACTIVE       = 1

######################################################################################

class MarlinOSD_Plugin(octoprint.plugin.StartupPlugin,
                       octoprint.plugin.TemplatePlugin,
                       octoprint.plugin.AssetPlugin,
                       octoprint.plugin.SimpleApiPlugin,
                       octoprint.plugin.SettingsPlugin):

    def on_after_startup(self):
        self._logger.info("             >> starting MarlinOSD plugin")
        # UGGLY, BUT WORKING !
        dir= self.get_template_folder() # ->  path to MarlinOSD/templates
        marlin_osd = "/" + dir + "/../bin/marlin_osd"
        #subprocess.run(marlin_osd)
        os.system(marlin_osd)

    ##~~ SettingsPlugin mixin

    def get_settings_defaults(self):
        return dict(
                        gfx_quality           = DEFAULT_GFX_QUALITY,
                        display_mode          = DEFAULT_DISPLAY_MODE,
                        marlin_mode_theme     = DEFAULT_MARLIN_MODE_THEME,
                        size                  = DEFAULT_SIZE,
                        position              = DEFAULT_POSITION,
                        fgnd_color            = DEFAULT_FGND_COLOR,
                        fgnd_opacity          = DEFAULT_FGND_OPACITY,
                        bkgnd_color           = DEFAULT_BKGND_COLOR,
                        bkgnd_opacity         = DEFAULT_BKGND_OPACITY,

                        # tab pane : System
                        show_at_startup       = DEFAULT_SHOW_AT_STARTUP,
                        disable_HID           = DEFAULT_DISABLE_HID,
                        demo_mode             = DEFAULT_DEMO_MODE,
                        spi_speed             = DEFAULT_SPI_SPEED,
                        dtr_timeout           = DEFAULT_DTR_TIMEOUT,
                        marlin_btn_debounce   = DEFAULT_MARLIN_BTN_DEBOUNCE,

                        # tab pane : Hardware
                        spi_dev               = DEFAULT_SPI_DEV,
                        dtr_pin               = DEFAULT_DTR_PIN,
                        dtr_pud               = DEFAULT_DTR_PUD,
                        dtr_edge              = DEFAULT_DTR_EDGE,
                        marlin_btn_pin        = DEFAULT_MARLIN_BTN_PIN,
                        marlin_btn_pud        = DEFAULT_MARLIN_BTN_PUD,
                        marlin_btn_edge       = DEFAULT_MARLIN_BTN_EDGE,
                        encoder_en_pin        = DEFAULT_ENCODER_EN_PIN,
                        encoder_en_active     = DEFAULT_ENCODER_EN_ACTIVE
        )

    def get_template_configs(self):
        return [
            dict(type="settings", custom_bindings=True)
        ]

#    def get_api_commands(self):
#        return dict(
#            applyNow = []
#        )

#    def on_api_command(self, command, data):
#        if command == "applyNow":
#            self._logger.info("############################## MarlinOSD : Apply Now ################################")

    ##~~ AssetPlugin mixin

    def get_assets(self):
        # Define your plugin's asset files to automatically include in the
        # core UI here.
        return {
            "js": ["js/MarlinOSD.js"],
            #"css": ["css/MarlinOSD.css"],
            #"less": ["less/MarlinOSD.less"]
        }

    ##~~ Softwareupdate hook

    def get_update_information(self):
        # Define the configuration for your plugin to use with the Software Update
        # Plugin here. See https://docs.octoprint.org/en/master/bundledplugins/softwareupdate.html
        # for details.
        return {
                "MarlinOSD": {
                "displayName": "Marlin OSD",
                "displayVersion": self._plugin_version,

                # version check: github repository
                "type": "github_release",
                "user": "you",
                "repo": "MarlinOSD",
                "current": self._plugin_version,

                # update method: pip
                "pip": "https://github.com/you/MarlinOSD/archive/{target_version}.zip",
            }
        }


# If you want your plugin to be registered within OctoPrint under a different name than what you defined in setup.py
# ("OctoPrint-PluginSkeleton"), you may define that here. Same goes for the other metadata derived from setup.py that
# can be overwritten via __plugin_xyz__ control properties. See the documentation for that.
__plugin_name__ = "Marlin OSD"


# Set the Python version your plugin is compatible with below. Recommended is Python 3 only for all new plugins.
# OctoPrint 1.4.0 - 1.7.x run under both Python 3 and the end-of-life Python 2.
# OctoPrint 1.8.0 onwards only supports Python 3.
__plugin_pythoncompat__ = ">=3,<4"  # Only Python 3

def __plugin_load__():
    global __plugin_implementation__
    __plugin_implementation__ = MarlinOSD_Plugin()

    global __plugin_hooks__
    __plugin_hooks__ = {
        "octoprint.plugin.softwareupdate.check_config": __plugin_implementation__.get_update_information
    }
