#include "pebble.h"

#define NUM_MENU_SECTIONS 2
#define NUM_FIRST_MENU_ITEMS 2
#define NUM_SECOND_MENU_ITEMS 1

SmartstrapAttribute *attribute;
static bool s_service_available;
// Pointer to the attribute buffer
size_t buff_size;
uint8_t *buffer;
static Window *s_main_window;
static SimpleMenuLayer *s_simple_menu_layer;
static SimpleMenuSection s_menu_sections[NUM_MENU_SECTIONS];
static SimpleMenuItem s_first_menu_items[NUM_FIRST_MENU_ITEMS];
static SimpleMenuItem s_second_menu_items[NUM_SECOND_MENU_ITEMS];
static GBitmap *s_menu_icon_image;
static GBitmap *lock;
static GBitmap *unlock;
static GBitmap *connected;
static GBitmap *disconnected;
static GBitmap *testing;

static bool smartlock_connected = false;

//CONNECTIVITY CALLBACK
//   Handler for when the "Status" option is selected
//
//   NEEDS TO BE REFINED TO DETECT ACTUAL CONNECTION
static void connectivity_callback(int index, void *ctx) {
  smartlock_connected = !smartlock_connected;
  
  if (smartlock_connected) 
  {
    s_second_menu_items[index].subtitle = "Connected!";
    s_second_menu_items[index].icon = connected;
  }
  else 
  {
    s_second_menu_items[index].subtitle = "Disconnected!";
    s_second_menu_items[index].icon = disconnected;
  }
  layer_mark_dirty(simple_menu_layer_get_layer(s_simple_menu_layer));

}

//LOCK SELECTION CALLBACK
//   Handler for when the "Lock Door" or "Unlock Door" options
//   are selected.
//
//   NEEDS TO BE REFINED TO ACTUALLY SEND COMMANDS AND 
//   ACT ACCORDINGLY
static void lock_selection_callback(int index, void *ctx) {

  SimpleMenuItem *menu_item = &s_first_menu_items[index];

  //If "Lock Door" is selected
  if (index == 0) 
  {
    menu_item->subtitle = "Command sent!";
    // Begin the write request, getting the buffer and its length
    smartstrap_attribute_begin_write(attribute, &buffer, &buff_size);
    // Store the data to be written to this attribute
    snprintf((char*)buffer, buff_size, "Lock Door");
    // End the write request, and send the data, not expecting a response
    smartstrap_attribute_end_write(attribute, buff_size, false);
  } 
  //If "Unlock Door" is selected
  else 
  {
    menu_item->subtitle = "Command sent!";
    // Begin the write request, getting the buffer and its length
    smartstrap_attribute_begin_write(attribute, &buffer, &buff_size);
    // Store the data to be written to this attribute
    snprintf((char*)buffer, buff_size, "Unlock Door");
    // End the write request, and send the data, not expecting a response
    smartstrap_attribute_end_write(attribute, buff_size, false);
  }
  
  layer_mark_dirty(simple_menu_layer_get_layer(s_simple_menu_layer));
}

static void main_window_load(Window *window) {
  //Create bitmaps of images for application
  lock = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_LOCK);
  unlock = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_UNLOCK);
  connected = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_CONNECTED);
  disconnected = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_DISCONNECTED);
  testing = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_TESTING);

  // Although we already defined NUM_FIRST_MENU_ITEMS, you can define
  // an int as such to easily change the order of menu items later
  int num_a_items = 0;

  s_menu_sections[0] = (SimpleMenuSection) {
    .title = PBL_IF_RECT_ELSE("Pebble SmartLock™", NULL),
    .num_items = NUM_FIRST_MENU_ITEMS,
    .items = s_first_menu_items,
  };
    
  s_first_menu_items[0] = (SimpleMenuItem) {
    .title = "Lock Door",
    .icon = lock,
    .subtitle = "Press to send",
    .callback = lock_selection_callback,
  };
  
  s_first_menu_items[1] = (SimpleMenuItem) {
    .title = "Unlock Door",
    .icon = unlock,
    .subtitle = "Press to send",
    .callback = lock_selection_callback,
  };

  s_menu_sections[1] = (SimpleMenuSection) {
    .title = PBL_IF_RECT_ELSE("Connectivity", NULL),
    .num_items = NUM_SECOND_MENU_ITEMS,
    .items = s_second_menu_items,
  };

  s_second_menu_items[num_a_items++] = (SimpleMenuItem) {
    .title = "Status",
    .subtitle = "Checking...",
    .callback = connectivity_callback,
    .icon = testing,
  };

  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_frame(window_layer);

  s_simple_menu_layer = simple_menu_layer_create(bounds, window, s_menu_sections, NUM_MENU_SECTIONS, NULL);

  layer_add_child(window_layer, simple_menu_layer_get_layer(s_simple_menu_layer));
}

void main_window_unload(Window *window) {
  simple_menu_layer_destroy(s_simple_menu_layer);
  gbitmap_destroy(s_menu_icon_image);
}

static void strap_availability_handler(SmartstrapServiceId service_id, bool is_available) {
  // A service's availability has changed
  APP_LOG(APP_LOG_LEVEL_INFO, "Service %d is %s available", (int)service_id, is_available ? "now" : "NOT");

  // Remember if the raw service is available
  s_service_available = (is_available && service_id == SMARTSTRAP_RAW_DATA_SERVICE_ID);
}

static void strap_notify_handler(SmartstrapAttribute *attribute) {
  // this will occur if the smartstrap notifies pebble
  //probably wont be used in our case
  vibes_short_pulse();
}

static void strap_write_handler(SmartstrapAttribute *attribute,
                                SmartstrapResult result) {
  // A write operation has been attempted
  if(result != SmartstrapResultOk) {
    // Handle the failure
    //APP_LOG(APP_LOG_LEVEL_ERROR, "Smartstrap error occured: %s", smartstrap_result_to_string(result));
  }
}

static void strap_init() {
  attribute = smartstrap_attribute_create(SMARTSTRAP_RAW_DATA_SERVICE_ID, SMARTSTRAP_RAW_DATA_ATTRIBUTE_ID, 64);

  // Subscribe to smartstrap events
  smartstrap_subscribe((SmartstrapHandlers) {
    .availability_did_change = strap_availability_handler,
    .notified = strap_notify_handler,
    .did_write = strap_write_handler,  
  }); 
}

static void init() {
  strap_init();
  
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
