
#include <pebble.h>
#include <pebble_fonts.h>
#define KEY_CONDITIONS 0
#define KEY_TEMPERATURE 1
#define KEY_FORECASTC 2
#define KEY_CITY 3
#define KEY_FORECASTT 4
#define KEY_DESC 5

static Window *s_main_window;  //For Window
static TextLayer *s_time_layer;   //For Text 
static GFont s_time_font;  //For custom font
static GFont s_battery_font;
static TextLayer *s_battery_layer;  //For battery percentage
static BitmapLayer *s_bt_layer; //For BT connection state
static GBitmap        *bluetooth_image; //For BT icon
static BitmapLayer  *s_w_layer; //For current weather
static GBitmap   *weather_image; //For current weather
static BitmapLayer  *s_nw_layer; //For next weather
static GBitmap   *n_weather_image; //For next weather
static TextLayer *s_temp_layer; //Temperature layer
static TextLayer *s_tempf_layer; //Forecast Temperature
static TextLayer *s_city_layer; //City layer
static TextLayer *s_desc_layer; //Current weather description
static char WeatherUpdateInterval[] = "30"; //Update interval
static TextLayer *s_day_label; //Week day and day of the month


// Previous bluetooth connection status
static bool prev_bt_status = false;

/*
  Handle bluetooth events
*/
void handle_bluetooth( bool connected ) {
  
  if ( bluetooth_image != NULL ) {
    gbitmap_destroy( bluetooth_image );
  }
  if ( connected ) {
    bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_YES_BT);
  } else {
    bluetooth_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_NO_BT);
    if ( prev_bt_status != connected) {
      vibes_short_pulse();
    }
 
  }

  prev_bt_status = connected;

  bitmap_layer_set_bitmap( s_bt_layer, bluetooth_image );
}


static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[100];

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "%d%% ", charge_state.charge_percent);
    text_layer_set_text_color(s_battery_layer, GColorWhite);
    text_layer_set_background_color(s_battery_layer, GColorBlack);
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%% ", charge_state.charge_percent);
    text_layer_set_text_color(s_battery_layer, GColorBlack);
    text_layer_set_background_color(s_battery_layer, GColorWhite);
  }
  text_layer_set_text(s_battery_layer, battery_text);
}


static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";
  static char minute[] = "00";
  static char num_buffer[4];
  static char day_buffer[6];
  static char date_buffer[10];
  
  // Write the current hours and minutes into the buffer
   
    strftime(buffer, sizeof(buffer), "%T", tick_time);
    strftime(minute, sizeof(minute), "%M", tick_time);
    strftime(num_buffer, sizeof(num_buffer), "%d", tick_time);
    strftime(day_buffer, sizeof(day_buffer), "%a", tick_time);
    snprintf(date_buffer, sizeof(date_buffer), "%s %s", day_buffer, num_buffer);
    text_layer_set_text(s_time_layer,"");
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
  text_layer_set_text(s_day_label, date_buffer);

  handle_battery(battery_state_service_peek());
  handle_bluetooth(bluetooth_connection_service_peek());
  

if (strcmp(minute, "00") == 0 || strcmp(minute, WeatherUpdateInterval) == 0) {
             
        // Begin dictionary
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);
        
        // Add a key-value pair
        dict_write_uint8(iter, 0, 0);
        
        // Send the message!
        app_message_outbox_send();
        
        APP_LOG(APP_LOG_LEVEL_INFO, "Weather Update requested");
 
 }


}



static void main_window_load(Window *window) 
{
  //Layer *window_layer = window_get_root_layer(window);
  //GRect window_bounds = layer_get_bounds(window_layer);
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
  s_battery_layer = text_layer_create(GRect(111, 4, 34, 17));
  text_layer_set_text_color(s_battery_layer, GColorBlack);
  s_battery_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERT_16));
  text_layer_set_font(s_battery_layer,s_battery_font);
  text_layer_set_text_alignment(s_battery_layer, GTextAlignmentRight);
  text_layer_set_text(s_battery_layer, "100% ");

  
  battery_state_service_subscribe(handle_battery);
  bluetooth_connection_service_subscribe(handle_bluetooth);

  //BT status layer 
  GRect BT_RECT        = GRect( 0,  4,  17,  20 );
  s_bt_layer = bitmap_layer_create(BT_RECT);

  
  //Temperature layer
  s_temp_layer = text_layer_create(GRect(1, 148, 24, 24));
  text_layer_set_text_color(s_temp_layer,GColorBlack);
  text_layer_set_font(s_temp_layer,fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERT_16)));
  text_layer_set_text_alignment(s_temp_layer, GTextAlignmentLeft);
  text_layer_set_text(s_temp_layer,"--");
  
  //Current weather
  GRect W_RECT = GRect(20, 148, 18, 17);
  s_w_layer = bitmap_layer_create(W_RECT);
  weather_image = gbitmap_create_with_resource(RESOURCE_ID_NA);
  bitmap_layer_set_bitmap( s_w_layer, weather_image );
  
  //Forecast
  GRect NW_RECT = GRect(105, 148, 18, 17);
  s_nw_layer = bitmap_layer_create(NW_RECT);
  n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_NA);
  bitmap_layer_set_bitmap(s_nw_layer, n_weather_image);
  
  //City
  s_city_layer = text_layer_create(GRect(22, 4, 100, 17));
  text_layer_set_text_color(s_city_layer,GColorBlack);
  text_layer_set_font(s_city_layer,fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERT_16)));
  text_layer_set_text_alignment(s_city_layer, GTextAlignmentCenter);
  text_layer_set_text(s_city_layer,"...");
  
  //Forecast temp
  s_tempf_layer = text_layer_create(GRect(122,148,21,24));
  text_layer_set_text_color(s_tempf_layer,GColorBlack);
  text_layer_set_font(s_tempf_layer,fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERT_16)));
  text_layer_set_text_alignment(s_tempf_layer, GTextAlignmentRight);
  text_layer_set_text(s_tempf_layer,"...");
  
  //Current weather desc
  s_desc_layer = text_layer_create(GRect(45,148,56,24));
  text_layer_set_text_color(s_desc_layer,GColorBlack);
  text_layer_set_font(s_desc_layer,fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_VERT_16)));
  text_layer_set_text_alignment(s_desc_layer, GTextAlignmentCenter);
  text_layer_set_text(s_desc_layer,"...");
  
  //Day
  s_day_label = text_layer_create(GRect(0,50,150,20));
  text_layer_set_text_color(s_day_label,GColorBlack);
  text_layer_set_font(s_day_label,fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CUSTOM_LIGHT_20)));
  text_layer_set_text_alignment(s_day_label, GTextAlignmentCenter);
  text_layer_set_text(s_day_label,"...");

  
  //update_time();
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_battery_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_bt_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temp_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_w_layer));
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_nw_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_city_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_tempf_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_desc_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_day_label));


  handle_battery(battery_state_service_peek());
  handle_bluetooth(bluetooth_connection_service_peek());
                                       
}

/*
  Destroy GBitmap and BitmapLayer
*/
void destroy_graphics( GBitmap *image, BitmapLayer *layer ) {
  layer_remove_from_parent( bitmap_layer_get_layer( layer ) );
  bitmap_layer_destroy( layer );
  if ( image != NULL ) {
    gbitmap_destroy( image );
  }
}

static void main_window_unload(Window *window) 
{
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_battery_layer);
  text_layer_destroy(s_temp_layer);
  text_layer_destroy(s_city_layer);
  text_layer_destroy(s_tempf_layer);
  text_layer_destroy(s_desc_layer);
  text_layer_destroy(s_day_label);

  
    // Unsubscribe from services
  tick_timer_service_unsubscribe();
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();

  // Destroy image objects
  destroy_graphics( bluetooth_image, s_bt_layer );
  destroy_graphics( weather_image, s_w_layer );
  destroy_graphics( n_weather_image, s_nw_layer);
  
    // Destroy font objects
  fonts_unload_custom_font( s_time_font );
  fonts_unload_custom_font( s_battery_font );
  

}
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) 
{
  update_time();
}


// APP MESSAGE
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
	static char temperature_buffer[25];
	static char conditions_buffer[25];
  static char forecast_buffer[25];
  static char city_buffer[100];
  static char tf_buffer[25];
  static char desc_buffer[100];
  
//		APP_LOG(APP_LOG_LEVEL_INFO, "Inbox Callback received !");

	// Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
		// Which key was received?
    switch(t->key) {
		case KEY_TEMPERATURE: 
        snprintf(temperature_buffer,sizeof(temperature_buffer),"%d",(int)t->value->int32);
      break;
		case KEY_CONDITIONS:
 		 	snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
     if (strcmp(conditions_buffer, "01d") == 0) {
        weather_image = gbitmap_create_with_resource(RESOURCE_ID_01D);
      } else if (strcmp(conditions_buffer, "01n") == 0) {
        weather_image = gbitmap_create_with_resource(RESOURCE_ID_01N);        
      } else if (strcmp(conditions_buffer, "02d") == 0) {
        weather_image = gbitmap_create_with_resource(RESOURCE_ID_02D);        
      } else if (strcmp(conditions_buffer, "02n") == 0) {
        weather_image = gbitmap_create_with_resource(RESOURCE_ID_02N);
      } else if (strcmp(conditions_buffer, "03d") == 0 || strcmp(conditions_buffer, "03n") == 0) {
        weather_image = gbitmap_create_with_resource(RESOURCE_ID_03D);
      } else if (strcmp(conditions_buffer, "04d") == 0 || strcmp(conditions_buffer, "04n") == 0) {
       weather_image = gbitmap_create_with_resource(RESOURCE_ID_04D);
      } else if (strcmp(conditions_buffer, "09d") == 0 || strcmp(conditions_buffer, "09n") == 0 || strcmp(conditions_buffer, "10d") == 0 || strcmp(conditions_buffer, "10n") == 0) {
       weather_image = gbitmap_create_with_resource(RESOURCE_ID_09D);
      } else if (strcmp(conditions_buffer, "11d") == 0 || strcmp(conditions_buffer, "11n") == 0) {
       weather_image = gbitmap_create_with_resource(RESOURCE_ID_11D);
      } else if (strcmp(conditions_buffer, "13d") == 0 || strcmp(conditions_buffer, "13n") == 0) {
       weather_image = gbitmap_create_with_resource(RESOURCE_ID_13D);
      } else if (strcmp(conditions_buffer, "50d") == 0 || strcmp(conditions_buffer, "50n") == 0) { 
       weather_image = gbitmap_create_with_resource(RESOURCE_ID_50D);
      } else {
       weather_image = gbitmap_create_with_resource(RESOURCE_ID_NA);
     }
    bitmap_layer_set_bitmap( s_w_layer, weather_image );
 		break;
    case KEY_FORECASTC:
      snprintf(forecast_buffer, sizeof(forecast_buffer), "%s", t->value->cstring);
     if (strcmp(forecast_buffer, "01d") == 0) {
        n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_01D);
      } else if (strcmp(forecast_buffer, "01n") == 0) {
        n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_01N);        
      } else if (strcmp(forecast_buffer, "02d") == 0) {
        n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_02D);        
      } else if (strcmp(forecast_buffer, "02n") == 0) {
        n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_02N);
      } else if (strcmp(forecast_buffer, "03d") == 0 || strcmp(forecast_buffer, "03n") == 0) {
        n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_03D);
      } else if (strcmp(forecast_buffer, "04d") == 0 || strcmp(forecast_buffer, "04n") == 0) {
       n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_04D);
      } else if (strcmp(forecast_buffer, "09d") == 0 || strcmp(forecast_buffer, "09n") == 0 || strcmp(forecast_buffer, "10d") == 0 || strcmp(forecast_buffer, "10n") == 0) {
       n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_09D);
      } else if (strcmp(forecast_buffer, "11d") == 0 || strcmp(forecast_buffer, "11n") == 0) {
       n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_11D);
      } else if (strcmp(forecast_buffer, "13d") == 0 || strcmp(forecast_buffer, "13n") == 0) {
       n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_13D);
      } else if (strcmp(forecast_buffer, "50d") == 0 || strcmp(forecast_buffer, "50n") == 0) { 
       n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_50D);
      } else {
       n_weather_image = gbitmap_create_with_resource(RESOURCE_ID_NA);
     }
      bitmap_layer_set_bitmap(s_nw_layer, n_weather_image);
    break;
    case KEY_CITY:
      snprintf(city_buffer, sizeof(city_buffer), "%s", t->value->cstring);
      //APP_LOG(APP_LOG_LEVEL_INFO, "City is %s", city_buffer);
      
    break;
    case KEY_FORECASTT:
      snprintf(tf_buffer, sizeof(tf_buffer), "%d", (int)t->value->int32);
    break;
    case KEY_DESC:
      snprintf(desc_buffer, sizeof(desc_buffer), "%s", t->value->cstring);
    break;
		default:
 		 APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
 		 break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }

		text_layer_set_text(s_temp_layer, temperature_buffer);
    text_layer_set_text(s_city_layer, city_buffer);
    text_layer_set_text(s_tempf_layer, tf_buffer);
    text_layer_set_text(s_desc_layer, desc_buffer);
} 

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "%d", reason);
	//text_layer_set_text(s_weather_layer, "Update Failed");
  //text_layer_set_text(s_forecast_layer, "Update Failed");
	text_layer_set_text(s_temp_layer, "U.F.");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
	//text_layer_set_text(s_weather_layer, "Update Failed"); // No BT, Internet
  //text_layer_set_text(s_forecast_layer, "Update Failed");
	text_layer_set_text(s_temp_layer, "Update Failed");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) { // updates weather
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

//TAP HANDLER
static void tap_handler(AccelAxisType axis, int32_t direction) {
	APP_LOG(APP_LOG_LEVEL_INFO, "Tap/flick registered!");
        DictionaryIterator *iter;
        app_message_outbox_begin(&iter);
        
        // Add a key-value pair
        dict_write_uint8(iter, 0, 0);
        
        // Send the message!
        app_message_outbox_send();
        
        APP_LOG(APP_LOG_LEVEL_INFO, "Weather Update requested");

}


// INIT
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
  
  // Tap handler
  accel_tap_service_subscribe(tap_handler);
  //Register with battery charge state service
// Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  

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


