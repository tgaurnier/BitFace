#include <pebble.h>

#define KEY_DATA 5
  
void *config_changed_callback(DictionaryIterator *config_iterator);

enum dateFormatter
{
  dmyyyy,
  ddmmyyyy,
  dmmm,
  dmmmyyyy,
  mmmd,
  mmmdyyyy,
  mdyyyy,
  mmddyyyy,
};

struct config
{
  bool coloursInverted;
  enum dateFormatter dateFomat;
  
};

static struct config app_config;

GColor get_foreground_colour()
{
  if(app_config.coloursInverted)
    return GColorBlack;
  else
    return GColorWhite;
}

GColor get_background_colour()
{
  if(app_config.coloursInverted)
    return GColorWhite;
  else
    return GColorBlack;
}

void get_date_formatter(char *buffer)
{
  switch(app_config.dateFomat)
  {
    case dateFormatter.dmyyyy :
      
      break;
      
    default:
      
      break;
  }
}



static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
//   // Get the first pair
//   Tuple *t = dict_read_first(iterator);

//   // Process all pairs present
//   while (t != NULL) {
//     // Long lived buffer
//     static char s_buffer[64];

//     // Process this pair's key
//     switch (t->key) {
//       case KEY_DATA:
//         // Copy value and display
//         snprintf(s_buffer, sizeof(s_buffer), "Received '%s'", t->value->cstring);
//         text_layer_set_text(s_output_layer, s_buffer);
//         break;
//     }

//     // Get next pair, if any
//     t = dict_read_next(iterator);
//   }
  
  config_changed_callback(iterator);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
  
void config_init(void *config_changed) {
  
  // Register callbacks
app_message_register_inbox_received(inbox_received_callback);
app_message_register_inbox_dropped(inbox_dropped_callback);
app_message_register_outbox_failed(outbox_failed_callback);
app_message_register_outbox_sent(outbox_sent_callback);


}



