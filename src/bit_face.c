/**
 * Bit Face
 * A binary watch face
 * Used "Just A Bit" as a Guide
 *
 * Copyright 2013 Tory Gaurnier
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
#include "app_config.h"

Window *window;
Layer *display_layer;
TextLayer *date_layer;
TextLayer *percent_layer;
BitmapLayer *battery_layer;
BitmapLayer *charge_layer;
InverterLayer *battfill_layer;
GBitmap *battery_outline;
GBitmap *battery_charge;
GFont font;
GFont font_tiny;

// Define circle and cell sizes
#define CIRCLE_RADIUS 					12
#define CIRCLE_LINE_THICKNESS			2
#define CIRCLE_PADDING					2
#define CELL_SIZE						(2 * (CIRCLE_RADIUS + CIRCLE_PADDING))
#define SIDE_PADDING					(144 - (4 * CELL_SIZE)) / 2
#define TOP_PADDING						10
// Define number of cells
#define CELLS_PER_ROW					4
#define CELLS_PER_COLUMN				4
// Define cell col offsets for each digit
#define HOURS_FIRST_DIGIT_COL			0
#define HOURS_SECOND_DIGIT_COL			1
#define MINUTES_FIRST_DIGIT_COL			2
#define MINUTES_SECOND_DIGIT_COL		3
// Max number of cell rows
#define DEFAULT_MAX_ROWS				4
#define HOURS_FIRST_DIGIT_MAX_ROWS		1 + clock_is_24h_style() // 2 rows if 24 hour time
#define MINUTES_FIRST_DIGIT_MAX_ROWS	3

static char percent_str[] = "xxx%";


static void draw_cell(GContext* context, GPoint center, bool filled) {
	// Each cell is a bit
	graphics_context_set_fill_color(context, GColorWhite);
	graphics_fill_circle(context, center, CIRCLE_RADIUS);

	if(!filled) {
		// This draws circles with a line thickness greater than 1 pixel
		graphics_context_set_fill_color(context, GColorBlack);
		graphics_fill_circle(context, center, CIRCLE_RADIUS - CIRCLE_LINE_THICKNESS);
	}
}


static GPoint get_center_point_from_cell_location(unsigned short x, unsigned short y) {
	return GPoint(
		SIDE_PADDING + (CELL_SIZE / 2) + (CELL_SIZE * x),
		TOP_PADDING + (CELL_SIZE / 2) + (CELL_SIZE * y)
	);
}


/**
 * Converts decimal into binary decimal form, then draws bits in cells
 */
static void draw_col(GContext* ctx, unsigned short digit, unsigned short max_rows, unsigned short col) {
	for(int i = 0; i < max_rows; i++)
		draw_cell(ctx, get_center_point_from_cell_location(col, i), (digit >> i) & 0x1);
}


/**
 * Converts 24hr to 12hr
 */
static unsigned short to_12_hour(unsigned short hour) {
	unsigned short display_hour = hour % 12;

	// If display_hour is 0, then set as 12
	return display_hour ? display_hour : 12;
}


static void display_layer_update_callback(Layer *me, GContext* context) {
	time_t now			=	time(NULL);
	struct tm *t		=	localtime(&now);
	unsigned short hour	=	t->tm_hour;
	unsigned short hour_first_digit;
	unsigned short hour_second_digit;

	if(!clock_is_24h_style()) {
		unsigned short period;
		if(hour < 12) period = 0; // 0 for AM
		else period = 1; // 1 for PM
		hour = to_12_hour(hour);
		hour_first_digit = period;
		hour_second_digit = hour;
	}

	else {
		hour_first_digit = hour / 10;
		hour_second_digit = hour % 10;
	}


	draw_col(
		context,
		hour_first_digit,
		HOURS_FIRST_DIGIT_MAX_ROWS,
		HOURS_FIRST_DIGIT_COL
	);
	draw_col(context,
		hour_second_digit,
		DEFAULT_MAX_ROWS,
		HOURS_SECOND_DIGIT_COL
	);

	draw_col(context,
		t->tm_min / 10,
		MINUTES_FIRST_DIGIT_MAX_ROWS,
		MINUTES_FIRST_DIGIT_COL
	);
	draw_col(context,
		t->tm_min % 10,
		DEFAULT_MAX_ROWS,
		MINUTES_SECOND_DIGIT_COL
	);
}


static void hide_battery() {
	layer_set_hidden(bitmap_layer_get_layer(battery_layer), true);
	layer_set_hidden(text_layer_get_layer(percent_layer), true);
	layer_set_hidden(inverter_layer_get_layer(battfill_layer), true);
}


static void show_battery() {
	layer_set_hidden(bitmap_layer_get_layer(battery_layer), false);
	layer_set_hidden(text_layer_get_layer(percent_layer), false);
	layer_set_hidden(inverter_layer_get_layer(battfill_layer), false);
}


static void set_battery(BatteryChargeState state) {
	// Set battery percent layer, account for bug where state.charge_percent never gets above 90
	if(state.is_plugged && !state.is_charging && state.charge_percent == 90)
		snprintf(percent_str, sizeof(percent_str), "100%%");
	else
		snprintf(percent_str, sizeof(percent_str), "%d%%", (int)state.charge_percent);
	text_layer_set_text(percent_layer, percent_str);

	// Set battery fill layer
	layer_set_frame(
		inverter_layer_get_layer(battfill_layer),
		GRect(8, 136, state.charge_percent/5, 16)
	);

	// Show or hide charging icon
	if(state.is_plugged)
		layer_set_hidden (bitmap_layer_get_layer(charge_layer), false);
	else
		layer_set_hidden(bitmap_layer_get_layer(charge_layer), true);

	// Show or hide battery indicator
	if(state.is_plugged || state.charge_percent <= 20)
		show_battery();
	else
		hide_battery();
}


static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
	static uint8_t count = 1;
	if(count == 1) {
		layer_mark_dirty(display_layer);

		static char date_text[] = "Xxxxxxxxx\n00/00/00";

		// TODO: Revert it back to "%A\n%m/%d/%y"
		strftime(date_text, sizeof(date_text), "%A\n%d/%m/%y", tick_time);
		text_layer_set_text(date_layer, date_text);
	}

	else if(count == 5) {
		BatteryChargeState state = battery_state_service_peek();
		if(!(state.is_plugged || state.charge_percent <= 20))
			hide_battery();
	}

	else if(count == 60)
		count = 0;

	count++;
}

static void config_changed(DictionaryIterator *iterator) {
  // Get the first pair
  Tuple *t = dict_read_first(iterator);

  // Process all pairs present
  while (t != NULL) {
    // Long lived buffer
    static char s_buffer[64];

    // Process this pair's key
    switch (t->key) {
      case KEY_DATA:
        // Copy value and display
        snprintf(s_buffer, sizeof(s_buffer), "Received '%s'", t->value->cstring);
        text_layer_set_text(date_layer, s_buffer); // using date_layer for testing
        break;
    }

    // Get next pair, if any
    t = dict_read_next(iterator);
  }
}

static void init(void) {
  
  // Initialise config reading
  config_init(config_changed);
  
	window = window_create();
	window_stack_push(window, true);
	window_set_background_color(window, GColorBlack);
	Layer *root_layer = window_get_root_layer(window);
	GRect frame = layer_get_frame(root_layer);

	// Init fonts and images
	font			=	fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_16));
	font_tiny		=	fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_10));
	battery_outline	=	gbitmap_create_with_resource(RESOURCE_ID_BATTERY_OUTLINE);
	battery_charge	=	gbitmap_create_with_resource(RESOURCE_ID_BATTERY_CHARGE);

	// Init layer for display
	display_layer = layer_create(frame);
	layer_set_update_proc(display_layer, &display_layer_update_callback);
	layer_add_child(root_layer, display_layer);

	// Init layer for text
	date_layer = text_layer_create(frame);
	layer_set_frame(text_layer_get_layer(date_layer), GRect(0, 130, 144, 168-130));
	layer_set_bounds(text_layer_get_layer(date_layer), GRect(0, 0, 144, 168-130));
	text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
	text_layer_set_font(date_layer, font);
	layer_add_child(root_layer, text_layer_get_layer(date_layer));

	// Init layer for battery image
	battery_layer = bitmap_layer_create(frame);
	bitmap_layer_set_background_color(battery_layer, GColorClear);
	layer_set_frame(bitmap_layer_get_layer(battery_layer), GRect(4, 134, 30, 20));
	layer_set_bounds(bitmap_layer_get_layer(battery_layer), GRect(0, 0, 30, 20));
	bitmap_layer_set_bitmap(battery_layer, battery_outline);
	layer_add_child(root_layer, bitmap_layer_get_layer(battery_layer));

	// Init layer for charge image
	charge_layer = bitmap_layer_create(frame);
	bitmap_layer_set_background_color(charge_layer, GColorClear);
	layer_set_frame(bitmap_layer_get_layer(charge_layer), GRect(8, 136, 20, 16));
	layer_set_bounds(bitmap_layer_get_layer(charge_layer), GRect(0, 0, 20, 16));
	bitmap_layer_set_bitmap(charge_layer, battery_charge);
	layer_add_child(root_layer, bitmap_layer_get_layer(charge_layer));

	// Init battery fill layer
	battfill_layer = inverter_layer_create(frame);
	layer_set_frame(inverter_layer_get_layer(battfill_layer), GRect(8, 136, 0, 16));
	layer_add_child(root_layer, inverter_layer_get_layer(battfill_layer));

	// Init layer for battery percentage
	percent_layer = text_layer_create(frame);
	layer_set_frame(text_layer_get_layer(percent_layer), GRect(4, 154, 30, 14));
	layer_set_bounds(text_layer_get_layer(percent_layer), GRect(0, 0, 30, 14));
	text_layer_set_text_alignment(percent_layer, GTextAlignmentCenter);
	text_layer_set_font(percent_layer, font_tiny);
	layer_add_child(root_layer, text_layer_get_layer(percent_layer));

	battery_state_service_subscribe(set_battery);
	set_battery(battery_state_service_peek());
	show_battery();

	tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);
}

static void deinit(void) {
	layer_destroy(display_layer);
	text_layer_destroy(date_layer);
	text_layer_destroy(percent_layer);
	bitmap_layer_destroy(battery_layer);
	bitmap_layer_destroy(charge_layer);
	inverter_layer_destroy(battfill_layer);
	fonts_unload_custom_font(font);
	fonts_unload_custom_font(font_tiny);
	gbitmap_destroy(battery_outline);
	gbitmap_destroy(battery_charge);
	tick_timer_service_unsubscribe();
	battery_state_service_unsubscribe();
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}