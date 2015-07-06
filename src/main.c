#include <pebble.h>
  
static Window *s_main_window;
static Layer *s_gfx_layer;
static TextLayer *s_time_layer;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  // formatting:  http://www.cplusplus.com/reference/ctime/strftime/
//   if(clock_is_24h_style() == true) {
//     // Use 24 hour format
//     strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
//   } else {
//     // Use 12 hour format
//     strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
//   }
  
  // Format the time as hexadecimal and place the string into the buffer:
  snprintf(buffer, sizeof("00:00"), "%X:%X", tick_time->tm_hour, tick_time->tm_min);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void graphics_update_proc(Layer *layer, GContext *ctx) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Update graphics");
    
    GRect bounds = layer_get_bounds(layer); // starts top - left
    GPoint center = grect_center_point(&bounds);
    
    int space = bounds.size.w / 8; // a bit dangerous
    int upperY = center.y / 2;
    int lowerY = center.y + upperY;
    
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Bounds: %d by %d", bounds.size.w, bounds.size.h); // 144 x 168
    
    // Get a tm structure
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);
    
    for(int i = space/2; i < bounds.size.w; i += space) {
        GPoint dot = {
            .x = i, // center.x -80,
            .y = upperY
        };
        graphics_draw_circle(ctx, dot, 4);
        
        GPoint dot2 = {
            .x = i,
            .y = lowerY
        };
        
        graphics_fill_circle(ctx, dot2, 4);
    }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    APP_LOG(APP_LOG_LEVEL_DEBUG, "time changed by %d", units_changed);
    // 2 for MINUTE_UNIT or 4 for HOUR_UNIT (you'll see 3 for sec and mins, or 7 for hours,min,secs)
    update_time();
}

static void main_window_load(Window *window) {
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    s_gfx_layer = layer_create(bounds);
    layer_set_update_proc(s_gfx_layer, graphics_update_proc);
    layer_add_child(window_layer, s_gfx_layer);
    
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
}

static void main_window_unload(Window *window) {
    // Destroy TextLayer
    text_layer_destroy(s_time_layer);
    
    // and the grpahics layer
    layer_destroy(s_gfx_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);


  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}