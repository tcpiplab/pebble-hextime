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
  
  // Format the time as hexadecimal and place the string into the buffer:
  snprintf(buffer, sizeof("00:00"), "%X:%X", tick_time->tm_hour, tick_time->tm_min);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void graphics_update_proc(Layer *layer, GContext *ctx) {
    //APP_LOG(APP_LOG_LEVEL_DEBUG, "Update graphics layer");
    
    GRect bounds = layer_get_bounds(layer); // starts top - left, original pebble is 144x168
    GPoint center = grect_center_point(&bounds);
    
    int s = bounds.size.w / 8; // space between each dot
    int p = s / 2; // padding
    int upperY = center.y / 2;
    int lowerY = center.y + upperY;
    
    // Get a tm structure
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);
    int hours = tick_time->tm_hour;
    int mins = tick_time->tm_min;
    
    // create a 2d array to hold the dots (2 rows of 8, row 0 for the hour bits and row 1 for minute bits)
    GPoint dots[2][8] = {
        { {.x=p, .y=upperY}, {.x=p+s, .y=upperY}, {.x=s*2+p, .y=upperY}, {.x=s*3+p, .y=upperY}, {.x=s*4+p, .y=upperY}, {.x=s*5+p, .y=upperY}, {.x=s*6+p, .y=upperY}, {.x=s*7+p, .y=upperY} },
        { {.x=p, .y=lowerY}, {.x=p+s, .y=lowerY}, {.x=s*2+p, .y=lowerY}, {.x=s*3+p, .y=lowerY}, {.x=s*4+p, .y=lowerY}, {.x=s*5+p, .y=lowerY}, {.x=s*6+p, .y=lowerY}, {.x=s*7+p, .y=lowerY} }
    };
    
    // loop backward so we can render dots from right to left
    for(int i = 7; i >= 0; i--) {        
        int y = 7-i; // sort out endian problem
        
        if( (hours >> i) & 0x01) {
            graphics_fill_circle(ctx, dots[0][y], 5); // the bit is set
        } else {
            graphics_draw_circle(ctx, dots[0][y], 3);
        }
        
        if( (mins >> i) & 0x01) {
            graphics_fill_circle(ctx, dots[1][y], 5); // the bit is set
        } else {
            graphics_draw_circle(ctx, dots[1][y], 3);
        }
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