#include <pebble.h>
#include "dot_matrix_watchface.h"

struct WatchFaceSetting SETTINGS = {
  .background = GColorWhite,
  .numerals_setting = {
    .color = GColorBlack
  },
  .grid_setting = {
    .color = GColorLightGray
  }
};

static struct ClaySettings cly_settings;

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

void clay_default_settings() {
  cly_settings.DisplayBackground = true;
  cly_settings.BackgroundStyle = PLUS;
  cly_settings.ClockStyle = false;
  cly_settings.NumeralsStyle = CIRCLE;
}

void clay_load_settings() {
  clay_default_settings();
  persist_read_data(SETTINGS_KEY, &cly_settings, sizeof(cly_settings));
}

void clay_save_settings() {
  persist_write_data(SETTINGS_KEY, &cly_settings, sizeof(cly_settings));
}

void populate_watchface_settings_from_clay_settings() {
  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "CLAY SETTINGS DisplayBackground=%i BackgroundStyle=%i ClockStyle=%i NumeralsStyle=%i",
    cly_settings.DisplayBackground,
    cly_settings.BackgroundStyle,
    cly_settings.ClockStyle,
    cly_settings.NumeralsStyle
  );
  struct SettingsMapping current_device_settings_mapping;
  struct ClockSettingMapping current_device_clock_settings_mapping;
  struct GridSetting grid_setting;
  struct DotSetting numerals_setting;

  switch(PBL_PLATFORM_TYPE_CURRENT) {
    case PlatformTypeFlint:
      current_device_settings_mapping = FLINT_SETTINGS_MAPPING;
      break;
    case PlatformTypeEmery:
      current_device_settings_mapping = EMERY_SETTINGS_MAPPING;
      break;
    case PlatformTypeGabbro:
      current_device_settings_mapping = GABBRO_SETTINGS_MAPPING;
      break;
    default:
      current_device_settings_mapping = FLINT_SETTINGS_MAPPING;
      break;
  }

  if (cly_settings.ClockStyle) {
    current_device_clock_settings_mapping = current_device_settings_mapping.inline_clock;
  } else {
    current_device_clock_settings_mapping = current_device_settings_mapping.noninline_clock;
  }

  switch(cly_settings.BackgroundStyle) {
    case PLUS:
      grid_setting = current_device_clock_settings_mapping.plus_grid;
      break;
    case DOT:
    default:
      grid_setting = current_device_clock_settings_mapping.dots_grid;
      break;
  }

  switch(cly_settings.NumeralsStyle) {
    case CIRCLE:
      numerals_setting = current_device_clock_settings_mapping.circle;
      break;
    case CIRCLE_OUTLINE:
      numerals_setting = current_device_clock_settings_mapping.circle_outline;
      break;
    case SQUARE:
      numerals_setting = current_device_clock_settings_mapping.square;
      break;
    case SQUARE_OUTLINE:
      numerals_setting = current_device_clock_settings_mapping.square_outline;
      break;
    case LINE:
      numerals_setting = current_device_clock_settings_mapping.line;
      break;
    case DOT:
    default:
      numerals_setting = current_device_clock_settings_mapping.dot;
      break;
  }
  
  if (cly_settings.DisplayBackground) {
    SETTINGS.display_grid = true;
  }
  
  SETTINGS.grid_spacing = current_device_clock_settings_mapping.grid_spacing;
  
  SETTINGS.grid_interval = grid_setting.interval;
  SETTINGS.grid_setting.r1 = grid_setting.dots.r1;
  SETTINGS.grid_setting.r2 = grid_setting.dots.r2;
  SETTINGS.grid_setting.shape = grid_setting.dots.shape;
  
  SETTINGS.numerals_setting.r1 = numerals_setting.r1;
  SETTINGS.numerals_setting.r2 = numerals_setting.r2;
  SETTINGS.numerals_setting.shape = numerals_setting.shape;

  APP_LOG(
    APP_LOG_LEVEL_INFO,
    "WATCHFACE SETTINGS background=%i foreground=%i grid_spacing=%i inline=%i",
    SETTINGS.background,
    SETTINGS.numerals_setting.color,
    SETTINGS.grid_spacing,
    SETTINGS.one_line_display
  );
}

void inbox_recieve_callback(DictionaryIterator *itr, void *ctx) {
  Tuple *background_style = dict_find(itr, MESSAGE_KEY_BackgroundStyle);
  Tuple *clock_style = dict_find(itr, MESSAGE_KEY_ClockStyle);
  Tuple *display_background = dict_find(itr, MESSAGE_KEY_DisplayBackground);
  Tuple *numerals_style = dict_find(itr, MESSAGE_KEY_NumeralsStyle);

  if (background_style) {
    cly_settings.BackgroundStyle = background_style->value->int32;
  }
  if (clock_style) {
    cly_settings.ClockStyle = clock_style->value->int32 == 1;
  }
  if (display_background) {
    cly_settings.DisplayBackground = display_background->value->int32 == 1;
  }
  if (numerals_style) {
    cly_settings.NumeralsStyle = numerals_style->value->int32;
  }
  if (background_style || clock_style || display_background || numerals_style) {
    clay_save_settings();
    populate_watchface_settings_from_clay_settings();
    layer_mark_dirty(dot_matrix_layer);
  }
}

void draw_dot(GContext *ctx, GPoint p, int r, int r2, GColor col, int shape) {
  graphics_context_set_fill_color(ctx, col);
  graphics_context_set_stroke_color(ctx, col);
  if (shape == SQUARE) {
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
    if (i % SETTINGS.grid_interval)
      continue;
    for (int j = 0; j*d + center.y < layer_size.h + r; j++) {
      if (j % SETTINGS.grid_interval)
        continue;
      draw_dot(ctx, GPoint(center.x + d*i, center.y + d*j), r, SETTINGS.grid_setting.r2, SETTINGS.grid_setting.color, SETTINGS.grid_setting.shape);
      if (j)
        draw_dot(ctx, GPoint(center.x + d*i, center.y - d*j), r, SETTINGS.grid_setting.r2, SETTINGS.grid_setting.color, SETTINGS.grid_setting.shape);
    }
    if (i) {
      for (int j = 0; j*d + center.y < layer_size.h + r; j++) {
        if (j % SETTINGS.grid_interval)
          continue;
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
    draw_dot(ctx, GPoint(center.x, center.y - SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);

    draw_dot(ctx, GPoint(center.x, center.y + SETTINGS.grid_spacing), 5, 0, SETTINGS.background, CIRCLE);
    draw_dot(ctx, GPoint(center.x, center.y + SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);

    draw_numeral(ctx, n1, GPoint(center.x - 8*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
    draw_numeral(ctx, n2, GPoint(center.x - 4*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
    draw_numeral(ctx, n3, GPoint(center.x + 2*SETTINGS.grid_spacing, center.y - 2*SETTINGS.grid_spacing), SETTINGS.numerals_setting.r1, SETTINGS.numerals_setting.r2, SETTINGS.grid_spacing, SETTINGS.numerals_setting.color, SETTINGS.numerals_setting.shape);
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

void refresh_time() {
  time_t t = time(NULL);
  struct tm *lt = localtime(&t);

  int h = (clock_is_24h_style()) ? lt->tm_hour : (lt->tm_hour % 12) ? lt->tm_hour % 12 : 12;
  int m = lt->tm_min;

  n1 = (h/10)%10;
  n2 = h%10;
  n3 = (m/10)%10;
  n4 = m%10;
}

// tick function
void tick(struct tm *tick_time, TimeUnits time_units) {
  refresh_time();
  layer_mark_dirty(dot_matrix_layer);
}

// window handlers
void window_load(Window *window) {
  // construct the window contents here
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  dot_matrix_layer = layer_create(bounds);
  layer_set_update_proc(dot_matrix_layer, draw_layer);

  layer_add_child(window_layer, dot_matrix_layer);

  populate_watchface_settings_from_clay_settings();
  refresh_time();
  layer_mark_dirty(dot_matrix_layer);
}

void window_unload(Window *window) {
  // destruct window contents here
  layer_destroy(dot_matrix_layer);
}

void init() {
  clay_load_settings();
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

  // set up inbox callback for reading watchface settings
  app_message_register_inbox_received(inbox_recieve_callback);
  app_message_open(128, 128);
}

void deinit() {
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
