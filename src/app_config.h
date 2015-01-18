#include <pebble.h>
// Include Pebble Autoconfig
#include "autoconfig.h"
  

typedef enum {
  dmyyyy,
  ddmmyyyy,
  dmmm,
  dmmmyyyy,
  mmmd,
  mmmdyyyy,
  mdyyyy,
  mmddyyyy,
} DateFormatter;

typedef struct  {
  bool colours_inverted;
  DateFormatter date_fromat;
  int battery_hide_seconds;
} config;

typedef void (*config_changed_callback)(config);

config get_current_config()
{
  config current_config;
  
  return current_config;
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
        // Let Pebble Autoconfig handle received settings
        autoconfig_in_received_handler(iter, context);

        // Here the updated settings are available
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Configuration updated. Background: %s Direction: %d Length: %d",
                getColours_inverted() ? "true" : "false", getDate_format(), (int)getBattery_hide_interval()); 
}

  
void config_init(config_changed_callback config_changed) {
  
        // Initialize Pebble Autoconfig to register App Message handlers and restores settings
        autoconfig_init();

        // Here the restored or defaulted settings are available
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Configuration updated. Background: %s Direction: %d Length: %d",
                getColours_inverted() ? "true" : "false", getDate_format(), (int)getBattery_hide_interval()); 

	// Call back wit current config
	config_changed(get_current_config());
	
        // Register our custom receive handler which in turn will call Pebble Autoconfigs receive handler
        app_message_register_inbox_received(in_received_handler);

    

}

void config_deinit() {
  
  // Let Pebble Autoconfig write settings to Pebbles persistant memory
  autoconfig_deinit();
}



