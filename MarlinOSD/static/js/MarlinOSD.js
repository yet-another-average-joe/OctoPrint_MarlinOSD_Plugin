/*
 * View model for MarlinOSD
 *
 * Author: Y@@J
 * License: AGPLv3
 */

$(function() {
    function MarlinOSD_ViewModel(parameters) {
            var self = this;

            // assign the injected parameters, e.g.:
            // self.loginStateViewModel = parameters[0];
            self.settingsViewModel = parameters[0];

            // TODO: Implement your plugin's view model here.
            self.applyNow = function (data) {
                OctoPrint.simpleApiCommand("MarlinOSD", "applyNow");
                //console.log("##################################### Apply Now #####################################");

            // a voir https://docs.octoprint.org/en/master/modules/plugin.html?highlight=save%20settings#octoprint.plugin.PluginSettings.save
            // save(force = False, trigger_event = False) avec True et False

            OctoPrint.settings.savePluginSettings('MarlinOSD', {
                // tab pane: Appearance
                'img_quality': self.settingsViewModel.settings.plugins.MarlinOSD.img_quality(),
                'display_mode': self.settingsViewModel.settings.plugins.MarlinOSD.display_mode(),
                'marlin_mode_theme': self.settingsViewModel.settings.plugins.MarlinOSD.marlin_mode_theme(),
                'size': self.settingsViewModel.settings.plugins.MarlinOSD.size(),
                'position': self.settingsViewModel.settings.plugins.MarlinOSD.position(),
                //'viewport_opacity': self.settingsViewModel.settings.plugins.MarlinOSD.viewport_opacity(), // not used, always 100%
                'fgnd_color': self.settingsViewModel.settings.plugins.MarlinOSD.fgnd_color(),
                'fgnd_opacity': self.settingsViewModel.settings.plugins.MarlinOSD.fgnd_opacity(),
                'bkgnd_color': self.settingsViewModel.settings.plugins.MarlinOSD.bkgnd_color(),
                'bkgnd_opacity': self.settingsViewModel.settings.plugins.MarlinOSD.bkgnd_opacity(),
                // tab pane: System
                'spi_speed': self.settingsViewModel.settings.plugins.MarlinOSD.spi_speed(),
                'debounce_time': self.settingsViewModel.settings.plugins.MarlinOSD.debounce_time(),
                'show_at_boot': self.settingsViewModel.settings.plugins.MarlinOSD.show_at_boot(),
                'disable_HID': self.settingsViewModel.settings.plugins.MarlinOSD.disable_HID(),
                'demo_mode': self.settingsViewModel.settings.plugins.MarlinOSD.demo_mode(),
            });
        }

        self.installService = function (data) {
            OctoPrint.simpleApiCommand("MarlinOSD", "installService");
            console.log('************************************ Install Service ***************************************')
        };

        self.uninstallService = function (data) {
            OctoPrint.simpleApiCommand("MarlinOSD", "uninstallService");
            console.log('************************************ Uninstall Service ***************************************')
        };
    }

    /* view model class, parameters for constructor, container to bind to
     * Please see http://docs.octoprint.org/en/master/plugins/viewmodels.html#registering-custom-viewmodels for more details
     * and a full list of the available options.
     */
    OCTOPRINT_VIEWMODELS.push({
        construct: MarlinOSD_ViewModel,
        // ViewModels your plugin depends on, e.g. loginStateViewModel, settingsViewModel, ...
        dependencies: [ /* "loginStateViewModel",*/ "settingsViewModel" ],
        // Elements to bind to, e.g. #settings_plugin_MarlinOSD, #tab_plugin_MarlinOSD, ...
        elements: [ "#settings_plugin_MarlinOSD" ]
    });
});
