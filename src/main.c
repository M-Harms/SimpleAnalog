#include <pebble.h>

#define COLORS       PBL_IF_COLOR_ELSE(true, false)
#define ANTIALIASING true

#define HOUR_HAND_MARGIN  13
#define MIN_HAND_MARGIN  10

#define FINAL_RADIUS 65
#define MIN_HAND_GREATER_LENGTH 10

#define ANIMATION_DURATION 0
#define ANIMATION_DELAY    0

typedef struct {
  int hours;
  int minutes;
} Time;

static Window *s_main_window;
static TextLayer *s_time_layer, *s_UTC_layer, *s_date_layer;
static Layer *s_canvas_layer;

static GPoint s_center;
static Time s_last_time, s_anim_time;
static int s_radius = 0, s_anim_hours_60 = 0, s_color_channels[3];
static bool s_animating = false;

static void bluetooth_callback(bool connected) {
  // Make text red on disconnect
  text_layer_set_text_color(s_date_layer, GColorWhite);
text_layer_set_text_color(s_UTC_layer, GColorWhite);

  
  if(!connected) {
    vibes_double_pulse();
    text_layer_set_text_color(s_date_layer, GColorRed);
    text_layer_set_text_color(s_UTC_layer, GColorRed);
    
  }
}



/*************************** AnimationImplementation **************************/

static void animation_started(Animation *anim, void *context) {
  s_animating = true;
}

static void animation_stopped(Animation *anim, bool stopped, void *context) {
  s_animating = false;
}

static void animate(int duration, int delay, AnimationImplementation *implementation, bool handlers) {
  Animation *anim = animation_create();
  animation_set_duration(anim, duration);
  animation_set_delay(anim, delay);
  animation_set_curve(anim, AnimationCurveEaseInOut);
  animation_set_implementation(anim, implementation);
  if(handlers) {
    animation_set_handlers(anim, (AnimationHandlers) {
      .started = animation_started,
      .stopped = animation_stopped
    }, NULL);
  }
  animation_schedule(anim);
}

/************************************ UI **************************************/

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  // Store time
  s_last_time.hours = tick_time->tm_hour;
  s_last_time.hours -= (s_last_time.hours > 12) ? 12 : 0;
  s_last_time.minutes = tick_time->tm_min;

  for(int i = 0; i < 3; i++) {
    s_color_channels[i] = rand() % 256;
  }

  // Redraw
  if(s_canvas_layer) {
    layer_mark_dirty(s_canvas_layer);
  }
}

static int hours_to_minutes(int hours_out_of_12) {
  return (int)(float)(((float)hours_out_of_12 / 12.0F) * 60.0F);
}

static void update_proc(Layer *layer, GContext *ctx) {
  
  

  int outside_buffer = 5; //Distance between reference and edge
  int degrees_between = 15; //Degrees between reference on the outside of the watch
  int current_degrees; //Degrees that is currently being drawn
  int current_number;
  int next_number; //The number of degrees neede to print the next number
 
    
  GRect bounds = layer_get_bounds(layer);  
  GRect outside_circle = GRect(outside_buffer,outside_buffer, bounds.size.w - outside_buffer*2, bounds.size.h - outside_buffer*2);
  
  
    // Color background?
  if(COLORS) {
    graphics_context_set_fill_color(ctx, GColorBlack);
  } else {
    graphics_context_set_fill_color(ctx, GColorDarkGray);
  }
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  graphics_context_set_stroke_color(ctx, GColorBlack);
  graphics_context_set_stroke_width(ctx, 7);

  graphics_context_set_antialiased(ctx, ANTIALIASING);

  // White clockface
 // graphics_context_set_fill_color(ctx, GColorBlue);
 // graphics_fill_circle(ctx, s_center, s_radius);

  //Draw Other watch items
  //graphics_fill_circle(ctx, s_center, s_radius)
  
  //Set color of outside references
 graphics_context_set_stroke_color(ctx, GColorGreen);
 graphics_context_set_fill_color(ctx, GColorWhite);
  
  //Set up do loop
  current_degrees = 30;
  current_number = 1;
  next_number = 30;
  
do { //Draw outside references until they go all the way around the circle
  //GRect point1 = grect_centered_from_polar(outside_circle,GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(current_degrees), GSize(10,10));
  
  //Draw Numbers
 // char watch_number [3];
  //snprintf(watch_number, 3,"%d", current_number);  
  //GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
  //GRect text_bounds = grect_centered_from_polar(outside_circle,GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(current_degrees), GSize(20,20));
  //graphics_context_set_text_color(ctx, GColorRed);
  //graphics_draw_text(ctx, watch_number, font, text_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
  
  
  if(current_degrees == next_number) {
   //Draw Numbers
      char watch_number [3];
      snprintf(watch_number, 3,"%d", current_number);  
      GFont font = fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD);
      GRect text_bounds = grect_centered_from_polar(outside_circle,GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(current_degrees), GSize(20,20));
      graphics_context_set_text_color(ctx, GColorWhite);
      graphics_draw_text(ctx, watch_number, font, text_bounds, GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
      next_number = next_number + 30;
      current_number = current_number + 1;    
}
else {
    //Draw Circles
  GPoint point1 = gpoint_from_polar(outside_circle,GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(current_degrees));
// graphics_draw_rect(ctx, point1); 
 graphics_fill_circle(ctx, point1, 1);
}
 
  
  
  //Add degrees for next iteration
  current_degrees = current_degrees + degrees_between;  
  
} while(current_degrees < 390);
 
 GColor TextColor;
   TextColor = GColorWhite;
  
//Add other time elements
  // Improve the layout to be more like a watchface for local time
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, TextColor);
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Improve the layout to be more like a watchface for UTC time
  text_layer_set_background_color(s_UTC_layer, GColorClear);
  //text_layer_set_text_color(s_UTC_layer, TextColor);
  text_layer_set_font(s_UTC_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_UTC_layer, GTextAlignmentCenter);
  
  // Create date TextLayer

//text_layer_set_text_color(s_date_layer, TextColor);
text_layer_set_background_color(s_date_layer, GColorClear);
text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  
  
  //Draw the other time functions
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);
  struct tm *tick_timeUTC = gmtime(&temp);
  

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);


//Do the same for UTC Time
  static char s_bufferUTC[9];
  strftime(s_bufferUTC, sizeof(s_bufferUTC),"%H:%M", tick_timeUTC);
  
//Get the date
  static char date_buffer[16];
strftime(date_buffer, sizeof(date_buffer), "%a, %b %d", tick_time);

  
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
 
  // Add the UTC label
  static char UTCTimeLabel[13] = "UTC: ";
   UTCTimeLabel[5]= '\0';
  strcat(UTCTimeLabel, s_bufferUTC);
 text_layer_set_text(s_UTC_layer, UTCTimeLabel);
  
  //Write the date
  text_layer_set_text(s_date_layer, date_buffer);
  

  // Draw outline
  //graphics_draw_circle(ctx, s_center, s_radius);
  
  // Don't use current time while animating
  Time mode_time = (s_animating) ? s_anim_time : s_last_time;

  // Adjust for minutes through the hour
  float minute_angle = TRIG_MAX_ANGLE * mode_time.minutes / 60;
  float hour_angle;
  if(s_animating) {
    // Hours out of 60 for smoothness
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 60;
  } else {
    hour_angle = TRIG_MAX_ANGLE * mode_time.hours / 12;
  }
  hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);

  // Plot hands
  GPoint minute_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius - MIN_HAND_MARGIN) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * mode_time.minutes / 60) * (int32_t)(s_radius - MIN_HAND_MARGIN) / TRIG_MAX_RATIO) + s_center.y,
  };
  GPoint hour_hand = (GPoint) {
    .x = (int16_t)(sin_lookup(hour_angle) * (int32_t)(s_radius - (2 * HOUR_HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.x,
    .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(s_radius - (2 * HOUR_HAND_MARGIN)) / TRIG_MAX_RATIO) + s_center.y,
  };

  // Draw hands with positive length only
  if(s_radius > 2 * HOUR_HAND_MARGIN) {
    graphics_context_set_stroke_color(ctx, GColorRed);
    graphics_draw_line(ctx, s_center, hour_hand);
  }
  if(s_radius > MIN_HAND_MARGIN) {
    graphics_context_set_stroke_color(ctx, GColorWhite);
    graphics_draw_line(ctx, s_center, minute_hand);
  }
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  

  s_center = grect_center_point(&window_bounds);

  s_canvas_layer = layer_create(window_bounds);
  
  layer_set_update_proc(s_canvas_layer, update_proc);
  layer_add_child(window_layer, s_canvas_layer);
  
  //Create Text layers
   // Get information about the Window


   // Create the TextLayer with specific bounds for local Time
  s_time_layer = text_layer_create(GRect(0, 40, window_bounds.size.w, 50));
  
    // Create the TextLayer with specific bounds for UTC Time
  s_UTC_layer = text_layer_create(GRect(0, 108, window_bounds.size.w, 50));
  
  //Create the Date Layer
  s_date_layer = text_layer_create(GRect(0, 90, window_bounds.size.w, 30));
  
  // Add it as a child layer to the Window's root layer
  //layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_UTC_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_date_layer));
  
  //Update Bluetooth
  bluetooth_callback(connection_service_peek_pebble_app_connection());
  
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas_layer);
}

/*********************************** App **************************************/

static int anim_percentage(AnimationProgress dist_normalized, int max) {
  return (int)(float)(((float)dist_normalized / (float)ANIMATION_NORMALIZED_MAX) * (float)max);
}

static void radius_update(Animation *anim, AnimationProgress dist_normalized) {
  s_radius = anim_percentage(dist_normalized, FINAL_RADIUS);

  layer_mark_dirty(s_canvas_layer);
}

static void hands_update(Animation *anim, AnimationProgress dist_normalized) {
  s_anim_time.hours = anim_percentage(dist_normalized, hours_to_minutes(s_last_time.hours));
  s_anim_time.minutes = anim_percentage(dist_normalized, s_last_time.minutes);

  layer_mark_dirty(s_canvas_layer);
}

static void init() {
  srand(time(NULL));

  time_t t = time(NULL);
  struct tm *time_now = localtime(&t);
  tick_handler(time_now, MINUTE_UNIT);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  // Prepare animations
  AnimationImplementation radius_impl = {
    .update = radius_update
  };
  animate(ANIMATION_DURATION, ANIMATION_DELAY, &radius_impl, false);

  AnimationImplementation hands_impl = {
    .update = hands_update
  };
  animate(2 * ANIMATION_DURATION, ANIMATION_DELAY, &hands_impl, true);
  
  //Register Bluetooth Service
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
  
}

static void deinit() {
  window_destroy(s_main_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}