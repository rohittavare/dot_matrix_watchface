#include <pebble.h>

// structs

struct DotSetting {
  int r1;
  int r2;
  int shape;
  GColor color;
};

struct WatchFaceSetting {
  int grid_spacing;
  GColor background;
  struct DotSetting numerals_setting;
  bool one_line_display;
  bool display_grid;
  struct DotSetting grid_setting;
};

// constants
const int CIRCLE = 0;
const int SQUARE = 1;
const int PLUS = 2;
const int LINE = 3;

const int NUMERALS_BITMASK[15] = {
  // top row
  0b0110111100,
  0b1111101111,
  0b0011110111,
  // second row
  0b1101110001,
  0b0000000000,
  0b1110011111,
  // third row
  0b1101100001,
  0b1111111100,
  0b1101010011,
  // fourth row
  0b0101000101,
  0b0010000000,
  0b1101111011,
  // fifth row
  0b1100101101,
  0b1111101101,
  0b0000010110
};

const struct WatchFaceSetting SETTINGS = {
  .grid_spacing = 15,
  .background = GColorBlack,
  .display_grid = true,
  .one_line_display = false,
  .numerals_setting = {
    .r1 = 3,
    .r2 = 2,
    .shape = LINE,
    .color = GColorWhite
  },
  .grid_setting = {
    .r1 = 3,
    .r2 = 1,
    .shape = PLUS,
    .color = GColorDarkGray
  }
};

// window
Window *main_window;

// layer(s)
Layer *dot_matrix_layer;

// other variables
int n1 = 0;
int n2 = 0;
int n3 = 0;
int n4 = 0;

// helpers

void draw_dot(GContext *ctx, GPoint p, int r, int r2, GColor col, int shape) {
  graphics_context_set_fill_color(ctx, col);
  graphics_context_set_stroke_color(ctx, col);
  if (shape == SQUARE) {
    graphics_fill_rect(ctx, GRect(p.x-r+1, p.y-r+1, 2*r, 2*r), 0, GCornerNone);
    if (r2) {
      for (int i = r2; i <= r; i++) {
        graphics_draw_rect(ctx, GRect(p.x-i+1, p.y-i+1, 2*i, 2*i));
      }
    } else {
      graphics_fill_rect(ctx, GRect(p.x-r+1, p.y-r+1, 2*r, 2*r), 0, GCornerNone);
    }
  } else if (shape == CIRCLE) {
    if (r2) {
      for (int i = r2; i <= r; i++) {
        graphics_draw_circle(ctx, p, i);
      }
    } else {
      graphics_fill_circle(ctx, p, r);
    }
  } else if (shape == PLUS) {
    graphics_fill_rect(ctx, GRect(p.x-r+1, p.y-(r2-1)/2, r*2-1, r2), 0, GCornerNone);
    graphics_fill_rect(ctx, GRect(p.x-(r2-1)/2, p.y-r+1, r2, r*2-1), 0, GCornerNone);
  } else if (shape == LINE) {
    graphics_fill_rect(ctx, GRect(p.x-(r2-1)/2, p.y-r+1, r2, r*2-1), 0, GCornerNone);
  }
}

void draw_numeral(GContext *ctx, int num, GPoint ul_coord, int r, int r2, int d, GColor c, int shape) {
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 5; j++) {
      if ((NUMERALS_BITMASK[i+j*3] >> num) % 2) {
        draw_dot(ctx, GPoint(ul_coord.x + d*i, ul_coord.y + d*j), r, r2, c, shape);
      }
    }
  }
}

void draw_grid(Layer *layer, GContext *ctx, int r, int d) {
  GSize layer_size = layer_get_bounds(layer).size;
  GPoint center = GPoint(layer_size.w/2, layer_size.h/2);

  for (int i = 0; i*d + center.x < layer_size.w + r; i++) {
    for (int j = 0; j*d + center.y < layer_size.h + r; j++) {
      draw_dot(ctx, GPoint(center.x + d*i, center.y + d*j), r, SETTINGS.grid_setting.r2, SETTINGS.grid_setting.color, SETTINGS.grid_setting.shape);
      if (j)
        draw_dot(ctx, GPoint(center.x + d*i, center.y - d*j), r, SETTINGS.grid_setting.r2, SETTINGS.grid_setting.color, SETTINGS.grid_setting.shape);
    }
    if (i) {
      for (int j = 0; j*d + center.y < layer_size.h + r; j++) {
        draw_dot(ctx, GPoint(center.x - d*i, center.y + d*j), r, SETTINGS.grid_setting.r2, SETTINGS.grid_setting.color, SETTINGS.grid_setting.shape);
        if (j)
          draw_dot(ctx, GPoint(center.x - d*i, center.y - d*j), r, SETTINGS.grid_setting.r2, SETTINGS.grid_setting.color, SETTINGS.grid_setting.shape);
      }
    }
  }
}


// layer draw functions
void draw_layer(Layer *layer, GContext *ctx) {
  GSize layer_size = layer_get_bounds(layer).size;
  GPoint center = GPoint(layer_size.w/2, layer_size.h/2);
  
  if (SETTINGS.display_grid)
    draw_grid(layer, ctx, SETTINGS.grid_setting.r1, SETTINGS.grid_spacing);
  if (SETTINGS.one_line_display) {
    draw_numeral(ctx, n1, GPoint(center.x - 8*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), 5, 0, SETTINGS.grid_spacing, SETTINGS.background, CIRCLE);
    draw_numeral(ctx, n2, GPoint(center.x - 4*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), 5, 0, SETTINGS.grid_spacing, SETTINGS.background, CIRCLE);
    draw_numeral(ctx, n3, GPoint(center.x + 2*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), 5, 0, SETTINGS.grid_spacing, SETTINGS.background, CIRCLE);
    draw_numeral(ctx, n4, GPoint(center.x + 6*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), 5, 0, SETTINGS.grid_spacing, SETTINGS.background, CIRCLE);

    draw_dot(ctx, GPoint(center.x, center.y - SETTINGS.grid_spacing), 5, 0, SETTINGS.background, CIRCLE);
    draw_dot(ctx, GPoint(center.x, center.y - SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.numerals_setting.color, CIRCLE);

    draw_dot(ctx, GPoint(center.x, center.y + SETTINGS.grid_spacing), 5, 0, SETTINGS.background, CIRCLE);
    draw_dot(ctx, GPoint(center.x, center.y + SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.numerals_setting.color, CIRCLE);

    draw_numeral(ctx, n1, GPoint(center.x - 8*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
    draw_numeral(ctx, n2, GPoint(center.x + 4*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
    draw_numeral(ctx, n3, GPoint(center.x - 2*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
    draw_numeral(ctx, n4, GPoint(center.x + 6*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
  } else {
    draw_numeral(ctx, n1, GPoint(center.x - 3*SETTINGS.grid_spacing, center.y - 5*SETTINGS.grid_spacing), 5, 0, SETTINGS.grid_spacing, SETTINGS.background, CIRCLE);
    draw_numeral(ctx, n2, GPoint(center.x + 1*SETTINGS.grid_spacing, center.y - 5*SETTINGS.grid_spacing), 5, 0, SETTINGS.grid_spacing, SETTINGS.background, CIRCLE);
    draw_numeral(ctx, n3, GPoint(center.x - 3*SETTINGS.grid_spacing, center.y + 1*SETTINGS.grid_spacing), 5, 0, SETTINGS.grid_spacing, SETTINGS.background, CIRCLE);
    draw_numeral(ctx, n4, GPoint(center.x + 1*SETTINGS.grid_spacing, center.y + 1*SETTINGS.grid_spacing), 5, 0, SETTINGS.grid_spacing, SETTINGS.background, CIRCLE);

    draw_numeral(ctx, n1, GPoint(center.x - 3*SETTINGS.grid_spacing, center.y - 5*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
    draw_numeral(ctx, n2, GPoint(center.x + 1*SETTINGS.grid_spacing, center.y - 5*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
    draw_numeral(ctx, n3, GPoint(center.x - 3*SETTINGS.grid_spacing, center.y + 1*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
    draw_numeral(ctx, n4, GPoint(center.x + 1*SETTINGS.grid_spacing, center.y + 1*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
  }
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
  window_set_background_color(main_window, SETTINGS.background);

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
