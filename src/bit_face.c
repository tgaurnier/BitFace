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

Window *window;
Layer *display_layer;
TextLayer *text_layer;

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

void draw_cell(GContext* context, GPoint center, bool filled) {
	// Each cell is a bit
	graphics_context_set_fill_color(context, GColorWhite);
	graphics_fill_circle(context, center, CIRCLE_RADIUS);

	if(!filled) {
		// This draws circles with a line thickness greater than 1 pixel
		graphics_context_set_fill_color(context, GColorBlack);
		graphics_fill_circle(context, center, CIRCLE_RADIUS - CIRCLE_LINE_THICKNESS);
	}
}


GPoint get_center_point_from_cell_location(unsigned short x, unsigned short y) {
	return GPoint(
		SIDE_PADDING + (CELL_SIZE / 2) + (CELL_SIZE * x),
		TOP_PADDING + (CELL_SIZE / 2) + (CELL_SIZE * y)
	);
}


/**
 * Converts decimal into binary decimal form, then draws bits in cells
 */
void draw_col(GContext* ctx, unsigned short digit, unsigned short max_rows, unsigned short col) {
	for(int i = 0; i < max_rows; i++)
		draw_cell(ctx, get_center_point_from_cell_location(col, i), (digit >> i) & 0x1);
}


/**
 * Converts 24hr to 12hr
 */
unsigned short to_12_hour(unsigned short hour) {
	unsigned short display_hour = hour % 12;

	// If display_hour is 0, then set as 12
	return display_hour ? display_hour : 12;
}


void display_layer_update_callback(Layer *me, GContext* context) {
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


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	layer_mark_dirty(display_layer);

	static char date_text[] = "Xxxxxxxxx\n00/00/00";

	strftime(date_text, sizeof(date_text), "%A\n%m/%d/%y", tick_time);
	text_layer_set_text(text_layer, date_text);
}

static void init(void) {
	window = window_create();
	window_stack_push(window, true);
	window_set_background_color(window, GColorBlack);
	Layer *root_layer = window_get_root_layer(window);
	GRect frame = layer_get_frame(root_layer);

	// Init layer for display
	display_layer = layer_create(frame);
	layer_set_update_proc(display_layer, &display_layer_update_callback);
	layer_add_child(root_layer, display_layer);

	// Init layer for text
	GFont font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_MY_FONT_16));
	text_layer = text_layer_create(frame);
	layer_set_frame((Layer*)text_layer, GRect(0, 130, 144, 168-130));
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
	layer_add_child(root_layer, (Layer*)text_layer);
	text_layer_set_font(text_layer, font);

	tick_timer_service_subscribe(MINUTE_UNIT, &handle_minute_tick);
}

static void deinit(void) {
	layer_destroy(display_layer);
	window_destroy(window);
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}