#include <pebble.h>

// window
Window *main_window;

// layer(s)
Layer *dot_matrix_layer;

// helpers

void draw_dot(GContext *ctx, GPoint p, int r, GColor col) {
  graphics_context_set_stroke_color(ctx, col);
  graphics_draw_circle(ctx, p, r);
}

void draw_grid(Layer *layer, GContext *ctx, int r, int d, GPoint offset) {
  GPoint true_offset = GPoint(
    (offset.x > d - r) ? offset.x - d + r : offset.x,
    (offset.y > d - r) ? offset.y - d + r : offset.y
  );
  GRect bounds = layer_get_bounds(layer);
  GPoint iter_b = GPoint(
    (bounds.size.w + r - true_offset.x)/d,
    (bounds.size.h + r - true_offset.y)/d
  );
  for (int i = 0; i <= iter_b.x; i++) {
    for (int j = 0; j <= iter_b.y; j++) {
      draw_dot(ctx, GPoint(true_offset.x + d*i, true_offset.y + d*j), r, GColorDarkGray);
    }
  }
}

// layer draw functions
void draw_layer(Layer *layer, GContext *ctx) {
  draw_grid(layer, ctx, 3, 12, GPoint(0, 0));
}

// tick function
void tick(struct tm *tick_time, TimeUnits time_units) {

}

// window handlers
void window_load(Window *window) {
  // construct the window contents here
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  dot_matrix_layer = layer_create(bounds);
  layer_set_update_proc(dot_matrix_layer, draw_layer);

  layer_add_child(window_layer, dot_matrix_layer);  
}

void window_unload(Window *window) {
  // destruct window contents here
  layer_destroy(dot_matrix_layer);
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
