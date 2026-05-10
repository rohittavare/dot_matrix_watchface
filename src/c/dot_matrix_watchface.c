#include <pebble.h>

// window
Window *main_window;

// layer(s)

// tick function
void tick(struct tm *tick_time, TimeUnits time_units) {

}

// window handlers
void window_load(Window *window) {
  // construct the window contents here
}

void window_unload(Window *window) {
  // destruct window contents here
}

void init() {
  main_window = window_create();
  window_set_background_color(main_window, GColorBlack);

  // set window load and unload handlers
  window_set_window_handlers(main_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload
  });

  // display the window
  window_stack_push(main_window, false);

  // register the tick function
  tick_timer_service_subscribe(MINUTE_UNIT, tick);
}

void deinit() {
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
