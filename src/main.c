#include <pebble.h>
#include "effect_layer.h"

Window *my_window;
TextLayer *text_layer_hours, *text_layer_minutes, *text_layer_dots;
EffectLayer *effect_layer_hours, *effect_layer_minutes;

char buffer_hours[] = "00";
char buffer_minutes[] = "00"; 
bool dots_inverted = true;

AppTimer *timer;

// utility function to create text layers
TextLayer * create_color_layer(GRect coords, char text[], GColor color, GColor bgcolor) {
  
  TextLayer * text_layer = text_layer_create(coords);
  text_layer_set_font(text_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_DUKE_97)));
  text_layer_set_text(text_layer, text);
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_text_color(text_layer, color);
  text_layer_set_background_color(text_layer, bgcolor);
  layer_add_child(window_get_root_layer(my_window), text_layer_get_layer(text_layer));
  
  return text_layer;
  
}

//timer callback function
void timer_callback(void *data) {
  
    Layer *layer = text_layer_get_layer(text_layer_dots);
  
    // hiding or showing dots
    if (dots_inverted) 
       layer_set_frame(layer, GRect(0, 0, 0 ,0));
    else  
       layer_set_frame(layer,GRect(52, -8, 50, 100));
  
    dots_inverted = !dots_inverted;
 
    timer = app_timer_register(1000, (AppTimerCallback) timer_callback, NULL);
}


// on time tick - set new values of minute & hour
void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed)
{
  
    if (!clock_is_24h_style()) {
    
        if( tick_time->tm_hour > 11 ) {   // YG Jun-25-2014: 0..11 - am 12..23 - pm
            if( tick_time->tm_hour > 12 ) tick_time->tm_hour -= 12;
        } else {
            if( tick_time->tm_hour == 0 ) tick_time->tm_hour = 12;
        }        
    }
 
    strftime(buffer_hours, sizeof("00"), "%H", tick_time);
    strftime(buffer_minutes, sizeof("00"), "%M", tick_time);
    
    text_layer_set_text(text_layer_hours, buffer_hours);
    text_layer_set_text(text_layer_minutes, buffer_minutes);
   
}  



void handle_init(void) {
  my_window = window_create();
  window_set_background_color(my_window, GColorBlack);
  window_stack_push(my_window, true);
 
  // creating text layers for hours & minutes
  text_layer_minutes = create_color_layer(GRect(15, -20, 110, 100), "00", GColorWhite, GColorClear);
  text_layer_hours = create_color_layer(GRect(15, 60, 110, 100), "00", GColorWhite, GColorClear);
  
  // creating effect layer to rotate minutes
  effect_layer_minutes = effect_layer_create(GRect(25,0,90,80));
  effect_layer_set_effect(effect_layer_minutes, EFFECT_ROTATE_LEFT );
  layer_add_child(window_get_root_layer(my_window), effect_layer_get_layer(effect_layer_minutes));
  
  // creating effect layer to rotate hours
  effect_layer_hours = effect_layer_create(GRect(25,80,90,80));
  effect_layer_set_effect(effect_layer_hours, EFFECT_ROTATE_LEFT );
  layer_add_child(window_get_root_layer(my_window), effect_layer_get_layer(effect_layer_hours));
  
  //creating text layer for dots
  text_layer_dots = create_color_layer(GRect(52, -8, 50, 100), "..", GColorWhite, GColorClear);
 
  //kicking of tick & timer services
  tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
  timer = app_timer_register(1000, (AppTimerCallback) timer_callback, NULL);
}

void handle_deinit(void) {
  text_layer_destroy(text_layer_hours);
  effect_layer_destroy(effect_layer_hours);
  effect_layer_destroy(effect_layer_minutes);
  window_destroy(my_window);
  
  tick_timer_service_unsubscribe();
  app_timer_cancel(timer);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
