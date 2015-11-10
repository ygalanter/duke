#include <pebble.h>
#include "effect_layer.h"

Window *my_window;
TextLayer *text_layer_hours, *text_layer_minutes, *text_layer_dots;
EffectLayer *effect_layer_hours, *effect_layer_minutes;

char buffer_hours[] = "00";
char buffer_minutes[] = "00"; 
bool dots_visible = true;

#ifndef PBL_SDK_2
static void app_focus_changed(bool focused) {
  if (focused) { // on resuming focus - restore background
    layer_mark_dirty(effect_layer_get_layer(effect_layer_hours));
  }
}
#endif


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


// on time tick - set new values of minute & hour
void handle_time_tick(struct tm *tick_time, TimeUnits units_changed)
{
  
    if (units_changed & SECOND_UNIT) { // on seconds change only blink dots
      
      if (dots_visible) 
         text_layer_set_text(text_layer_dots, " ");
      else  
         text_layer_set_text(text_layer_dots, "..");
    
      dots_visible = !dots_visible;
      
    }
  
    if (units_changed & MINUTE_UNIT) { // on minutes change - change minutes
      strftime(buffer_minutes, sizeof("00"), "%M", tick_time);
      text_layer_set_text(text_layer_minutes, buffer_minutes);  
    }  
  
    if (units_changed & HOUR_UNIT) { // on hours change - change hours
        
      if (!clock_is_24h_style()) {
            if( tick_time->tm_hour > 11 ) {   // YG Jun-25-2014: 0..11 - am 12..23 - pm
                if( tick_time->tm_hour > 12 ) tick_time->tm_hour -= 12;
            } else {
                if( tick_time->tm_hour == 0 ) tick_time->tm_hour = 12;
            }        
        }
     
        strftime(buffer_hours, sizeof("00"), "%H", tick_time);
        text_layer_set_text(text_layer_hours, buffer_hours);
    }  
  
}  



void handle_init(void) {
  
    #ifndef PBL_SDK_2
  // need to catch when app resumes focus after notification, otherwise background won't restore
  app_focus_service_subscribe_handlers((AppFocusHandlers){
    .did_focus = app_focus_changed
  });
  #endif
  
  
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
  
  //Get a time structure so that the face doesn't start blank
  time_t temp = time(NULL);
  struct tm *t = localtime(&temp);
 
  //Manually call the tick handler when the window is loading
  handle_time_tick(t, HOUR_UNIT | MINUTE_UNIT);
  
 
  //kicking of tick service
  tick_timer_service_subscribe(SECOND_UNIT, handle_time_tick);
  
}

void handle_deinit(void) {
  
  #ifndef PBL_SDK_2
    app_focus_service_unsubscribe();
  #endif
  
  text_layer_destroy(text_layer_hours);
  text_layer_destroy(text_layer_minutes);
  text_layer_destroy(text_layer_dots);
  
  effect_layer_destroy(effect_layer_hours);
  effect_layer_destroy(effect_layer_minutes);
  window_destroy(my_window);
  
  tick_timer_service_unsubscribe();
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
