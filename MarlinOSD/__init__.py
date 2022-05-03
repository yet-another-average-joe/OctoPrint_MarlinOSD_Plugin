# coding=utf-8
from __future__ import absolute_import

#
#
#
#       Marlin On Screen Display (MarlinOSD) __init__.py
#
#
#

import octoprint.plugin
#from octoprint.events import Events
#import logging
#import flask

# image quality (oversampling)
QLTY_LOW=0
QLTY_MEDIUM=1
QLTY_HIGH=2

# mode
DISPLAY_MODE_MARLIN=0
DISPLAY_MODE_CUSTOM=1

# position
TOP_LEFT=0
TOP_CENTER=1
TOP_RIGHT=2
CENTER_LEFT=3
CENTER_CENTER=4
CENTER_RIGHT=5
BOTTOM_LEFT=6
BOTTOM_CENTER=7
BOTTOM_RIGHT=8

class MarlinOSD_Plugin(octoprint.plugin.StartupPlugin,
                       octoprint.plugin.TemplatePlugin,
                       octoprint.plugin.AssetPlugin,
                       octoprint.plugin.SimpleApiPlugin,
                       octoprint.plugin.SettingsPlugin):

#    def on_after_startup(self):
#        self._logger.info("####################### MarlinOSD : on_after_startup(self) ##########################")

    ##~~ SettingsPlugin mixin

    def get_settings_defaults(self):
#        self._logger.info("##################### MarlinOSD : get_settings_defaults(self) #######################")
        return dict(
                        # tab pane : Appearance
                        img_quality         = QLTY_MEDIUM,
                        display_mode        = DISPLAY_MODE_CUSTOM,
                        marlin_mode_theme   = 0,
                        size                = 50,
                        position            = CENTER_CENTER,
                        #viewport_opacity    = 100, # not used, always 100%
                        fgnd_color          = "#ffffff",	# white
                        fgnd_opacity        = 100,
                        bkgnd_color         = "#0000ff",	# blue
                        bkgnd_opacity       = 100,
                        # tab pane : System
                        spi_speed           = 8,
                        debounce_time       = 300,
                        show_at_boot        = "true",
                        disable_HID         = "false",
                        demo_mode           = "true"
        )

    def get_template_configs(self):
#        self._logger.info("##################### MarlinOSD : get_template_configs(self) ########################")
        return [
            dict(type="settings", custom_bindings=True)
        ]

#    def get_api_commands(self):
#        self._logger.info("###################### MarlinOSD : get_api_commands(self) ###########################")
#        return dict(
#            applyNow = []
#        )

#    def on_api_command(self, command, data):
#        self._logger.info("################ MarlinOSD : on_api_command(self, command, data) ####################")
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
