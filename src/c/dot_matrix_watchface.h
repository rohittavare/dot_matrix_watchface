#include <pebble.h>

// constants
// to indicate shapes
typedef enum Shape {
  CIRCLE,
  SQUARE,
  PLUS,
  LINE,
  CIRCLE_OUTLINE,
  SQUARE_OUTLINE,
  DOT
} Shape;

// structs

#define SETTINGS_KEY 1

// the simplified settings choices made available in pebble app
struct ClaySettings {
  bool ClockStyle;
  bool DisplayBackground;
  Shape BackgroundStyle;
  Shape NumeralsStyle;
};
// ==========

struct DotSetting {
  int r1;
  int r2;
  Shape shape;
  GColor color;
};

struct GridSetting {
  int interval;
  struct DotSetting dots;
};

struct ClockSettingMapping {
  int grid_spacing;

  struct GridSetting dots_grid;
  struct GridSetting plus_grid;

  struct DotSetting circle;
  struct DotSetting circle_outline;
  struct DotSetting square;
  struct DotSetting square_outline;
  struct DotSetting dot;
  struct DotSetting line;
};

struct SettingsMapping {
  struct ClockSettingMapping noninline_clock;
  struct ClockSettingMapping inline_clock;
};

// used to inform what to draw on the actual watchface
struct WatchFaceSetting {
  int grid_spacing;
  int grid_interval;
  GColor background;
  struct DotSetting numerals_setting;
  bool one_line_display;
  bool display_grid;
  struct DotSetting grid_setting;
};

// Represents bitmasks for each numeral
//
// elements in this array represent pixels in a 3x4 grid
// proceeding from left to right, top to bottom
// starting at top left pixel
//
// each pixel is represented as a 10-bit string
// the leftmost bit represent whether that pixel is part of the mask for '9'
// the rightmost bit represents whether that pixel is part of the mask for '0'
// bits in between represents the mask status for that pixel '8-1'
const int NUMERALS_BITMASK[15] = {
  // top row
  0b0010111000,
  0b1111101101,
  0b0111110111,
  // second row
  0b1101110101,
  0b0000000010,
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
  0b1110101101,
  0b1101101101,
  0b0000010110
};

// below contain hand-picked values for different configurations of watchface settings
// each supported platform contain values for each type of watchface setting
const struct SettingsMapping FLINT_SETTINGS_MAPPING = {
  .inline_clock = {
    .grid_spacing = 8,
    .dots_grid = {
      .interval = 6,
      .dots = {
        .r1 = 1,
        .r2 = 0,
        .shape = SQUARE
      }
    },
    .plus_grid = {
      .interval = 6,
      .dots = {
        .r1 = 3,
        .r2 = 1,
        .shape = PLUS
      }
    },
    .circle = {
      .r1 = 3,
      .r2 = 0,
      .shape = CIRCLE
    },
    .circle_outline = {
      .r1 = 3,
      .r2 = 3,
      .shape = CIRCLE
    },
    .dot = {
      .r1 = 2,
      .r2 = 0,
      .shape = CIRCLE
    },
    .square = {
      .r1 = 3,
      .r2 = 0,
      .shape = SQUARE
    },
    .square_outline = {
      .r1 = 3,
      .r2 = 3,
      .shape = SQUARE
    },
    .line = {
      .r1 = 3,
      .r2 = 3,
      .shape = LINE
    }
  },
  .noninline_clock = {
    .grid_spacing = 10,
    .dots_grid = {
      .interval = 6,
      .dots = {
        .r1 = 1,
        .r2 = 0,
        .shape = SQUARE
      }
    },
    .plus_grid = {
      .interval = 6,
      .dots = {
        .r1 = 3,
        .r2 = 1,
        .shape = PLUS
      }
    },
    .circle = {
      .r1 = 3,
      .r2 = 0,
      .shape = CIRCLE
    },
    .circle_outline = {
      .r1 = 3,
      .r2 = 3,
      .shape = CIRCLE
    },
    .dot = {
      .r1 = 2,
      .r2 = 0,
      .shape = CIRCLE
    },
    .square = {
      .r1 = 3,
      .r2 = 0,
      .shape = SQUARE
    },
    .square_outline = {
      .r1 = 3,
      .r2 = 3,
      .shape = SQUARE
    },
    .line = {
      .r1 = 3,
      .r2 = 3,
      .shape = LINE
    }
  }
};

const struct SettingsMapping EMERY_SETTINGS_MAPPING = {
  .inline_clock = {
    .grid_spacing = 10,
    .dots_grid = {
      .interval = 3,
      .dots = {
        .r1 = 1,
        .r2 = 0,
        .shape = SQUARE
      }
    },
    .plus_grid = {
      .interval = 6,
      .dots = {
        .r1 = 5,
        .r2 = 1,
        .shape = PLUS
      }
    },
    .circle = {
      .r1 = 3,
      .r2 = 0,
      .shape = CIRCLE
    },
    .circle_outline = {
      .r1 = 3,
      .r2 = 2,
      .shape = CIRCLE
    },
    .dot = {
      .r1 = 2,
      .r2 = 0,
      .shape = CIRCLE
    },
    .square = {
      .r1 = 3,
      .r2 = 0,
      .shape = SQUARE
    },
    .square_outline = {
      .r1 = 3,
      .r2 = 2,
      .shape = SQUARE
    },
    .line = {
      .r1 = 3,
      .r2 = 3,
      .shape = LINE
    }
  },
  .noninline_clock = {
    .grid_spacing = 15,
    .dots_grid = {
      .interval = 3,
      .dots = {
        .r1 = 1,
        .r2 = 0,
        .shape = SQUARE
      }
    },
    .plus_grid = {
      .interval = 6,
      .dots = {
        .r1 = 5,
        .r2 = 1,
        .shape = PLUS
      }
    },
    .circle = {
      .r1 = 5,
      .r2 = 0,
      .shape = CIRCLE
    },
    .circle_outline = {
      .r1 = 5,
      .r2 = 4,
      .shape = CIRCLE
    },
    .dot = {
      .r1 = 2,
      .r2 = 0,
      .shape = CIRCLE
    },
    .square = {
      .r1 = 3,
      .r2 = 0,
      .shape = SQUARE
    },
    .square_outline = {
      .r1 = 3,
      .r2 = 3,
      .shape = SQUARE
    },
    .line = {
      .r1 = 3,
      .r2 = 3,
      .shape = LINE
    }
  }
};

const struct SettingsMapping GABBRO_SETTINGS_MAPPING = {
  .inline_clock = {
    .grid_spacing = 12,
    .dots_grid = {
      .interval = 2,
      .dots = {
        .r1 = 1,
        .r2 = 0,
        .shape = SQUARE
      }
    },
    .plus_grid = {
      .interval = 3,
      .dots = {
        .r1 = 5,
        .r2 = 1,
        .shape = PLUS
      }
    },
    .circle = {
      .r1 = 5,
      .r2 = 0,
      .shape = CIRCLE
    },
    .circle_outline = {
      .r1 = 5,
      .r2 = 4,
      .shape = CIRCLE
    },
    .dot = {
      .r1 = 2,
      .r2 = 0,
      .shape = CIRCLE
    },
    .square = {
      .r1 = 5,
      .r2 = 0,
      .shape = SQUARE
    },
    .square_outline = {
      .r1 = 5,
      .r2 = 4,
      .shape = SQUARE
    },
    .line = {
      .r1 = 3,
      .r2 = 3,
      .shape = LINE
    }
  },
  .noninline_clock = {
    .grid_spacing = 15,
    .dots_grid = {
      .interval = 2,
      .dots = {
        .r1 = 1,
        .r2 = 0,
        .shape = SQUARE
      }
    },
    .plus_grid = {
      .interval = 4,
      .dots = {
        .r1 = 5,
        .r2 = 1,
        .shape = PLUS
      }
    },
    .circle = {
      .r1 = 6,
      .r2 = 0,
      .shape = CIRCLE
    },
    .circle_outline = {
      .r1 = 5,
      .r2 = 5,
      .shape = CIRCLE
    },
    .dot = {
      .r1 = 2,
      .r2 = 0,
      .shape = CIRCLE
    },
    .square = {
      .r1 = 6,
      .r2 = 0,
      .shape = SQUARE
    },
    .square_outline = {
      .r1 = 5,
      .r2 = 5,
      .shape = SQUARE
    },
    .line = {
      .r1 = 3,
      .r2 = 3,
      .shape = LINE
    }
  }
};