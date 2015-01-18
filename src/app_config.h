/**
 * Bit Face
 * A binary watch face
 * Used "Just A Bit" as a Guide
 * Used Pebble-Autoconfig for configuration
 *
 * Copyright 2014 Renjith I S
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


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
} DateFormatter;		// TODO: change this to all_small_letters

typedef struct  {
	bool colours_inverted;
	DateFormatter date_fromat;
	int battery_hide_seconds;
} config;

typedef void (*config_changed_callback)(config);

config get_current_config();
void get_date_formatter(char* buffer);
void config_init(config_changed_callback config_changed);
void config_deinit();

config_changed_callback config_changed_function;		// Any advantage/disadvantage by using a static pointer?

void get_date_formatter(char* buffer) {
    config current_config=get_current_config();
    switch (current_config.date_fromat)
    {
	  /* Ref  : http://linux.die.net/man/3/strftime
	   * Ref2 : http://www.cplusplus.com/reference/ctime/strftime/ */
	  case dmyyyy:
		strcpy(buffer, "%A\n%e/%m/%Y");
		break;
	  case ddmmyyyy:
		strcpy(buffer, "%A\n%d/%m/%Y");
		break;
	  case dmmm:
		strcpy(buffer, "%A\n%e %b");
		break;
	  case dmmmyyyy:
		strcpy(buffer, "%A\n%d %b %Y");
		break;
	  case mmmd:
		strcpy(buffer, "%A\n%b %d");
		break;
	  case mmmdyyyy:
		strcpy(buffer, "%A\n%b %d %Y");
		break;
	  case mdyyyy:
		strcpy(buffer, "%A\n%m/%e/%Y");
		break;
	  case mmddyyyy:
		strcpy(buffer, "%A\n%m/%d/%Y");
		break;
		/*
	  case dmyyyy:
		strcpy(buffer, "%A\n");
		break;
	  case dmyyyy:
		strcpy(buffer, "%A\n");
		break;
	  case dmyyyy:
		strcpy(buffer, "%A\n");
		break;
		*/
	  default:
	  	strcpy(buffer, "%A\n%e/%m/%Y");
		break;
  }
}

config get_current_config() {
	config current_config;

	current_config.colours_inverted=getColours_inverted();
	current_config.date_fromat=getDate_format();
	current_config.battery_hide_seconds=getBattery_hide_interval();

	return current_config;
}

static void in_received_handler(DictionaryIterator *iter, void *context) {
	// Let Pebble Autoconfig handle received settings
	autoconfig_in_received_handler(iter, context);

	// Here the updated settings are available
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Configuration updated. Invert: %s DateFormat: %d Battery Hide in : %d",
			getColours_inverted() ? "true" : "false", getDate_format(), (int)getBattery_hide_interval()); 
	
	if(config_changed_function)
		config_changed_function(get_current_config());
}
  
void config_init(config_changed_callback config_changed) {
	// Initialize Pebble Autoconfig to register App Message handlers and restores settings
	autoconfig_init();

	// Here the restored or defaulted settings are available
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Configuration retrieved. Invert: %s DateFormat: %d Battery Hide in : %d",
			getColours_inverted() ? "true" : "false", getDate_format(), (int)getBattery_hide_interval()); 

	config_changed_function = config_changed;
	// Call back wit current config
	//config_changed(get_current_config());
	
	// Register our custom receive handler which in turn will call Pebble Autoconfigs receive handler
	app_message_register_inbox_received(in_received_handler);
}

void config_deinit() {
	// app_message_register_inbox_received will be deregistered by autoconfig using app_message_deregister_callbacks()

	// Let Pebble Autoconfig write settings to Pebbles persistant memory
	autoconfig_deinit();
}
