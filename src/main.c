
#include <pebble.h>
#include <pebble_fonts.h>

static Window *s_main_window;  //For Window
static TextLayer *s_time_layer;   //For Text 
static GFont s_time_font;  //For custom font
static GFont s_battery_font;
static TextLayer *s_battery_layer;  //For battery percentage

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%         ";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "Plugged %d%% ", charge_state.charge_percent);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% ", charge_state.charge_percent);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00AM";
  
  // Write the current hours and minutes into the buffer
   
    strftime(buffer, sizeof("00:00AM"), "%I:%M%p", tick_time);
    text_layer_set_text(s_time_layer,"");
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  handle_battery(battery_state_service_peek());

}





static void main_window_load(Window *window) 
{
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 80, 150, 35));
  //s_battery_layer=text_layer_create(GRect(0,0,150,100));
  
  text_layer_set_background_color(s_time_layer, GColorBlack);
  //window_set_background_color(window,GColorClear);
  text_layer_set_text_color(s_time_layer, GColorClear);
  

  
  // Improve the layout to be more like a watchface
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CUSTOM_LIGHT_24));
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  //Improve battery layer
  s_battery_layer = text_layer_create(GRect(0, 4, window_bounds.size.w, 34));
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERT_16));
  text_layer_set_font(s_battery_layer,s_battery_font);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text(s_battery_layer, "100% ");

  
  
  battery_state_service_subscribe(handle_battery);

  //update_time();
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));

  handle_battery(battery_state_service_peek());
                                       
}

static void main_window_unload(Window *window) 
{
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_battery_layer);
}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) 
{
  update_time();
}
static void init() 
{
   // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  // Make sure the time is displayed from the start
  update_time();
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  //Register with battery charge state service
  

}

static void deinit() 
{
  // Destroy Window
    window_destroy(s_main_window);
}

int main(void) 
{
  init();
  app_event_loop();
  deinit();
}