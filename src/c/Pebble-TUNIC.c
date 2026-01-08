#include <pebble.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

static Window *window;
static Layer *background_layer;
static GFont font;
static BitmapLayer *fox_layer;
static GBitmap *fox_bitmap;
static BitmapLayer *border_layer;
static GBitmap *border_bitmap;
static TextLayer *time_shadow_layer;
static TextLayer *time_layer;
static Layer *bars_layer;


enum BarColor {
  RED,
  GREEN,
  BLUE,
  BW
};


static void byte_set_bit(uint8_t *byte, uint8_t bit, uint8_t value) {
  *byte ^= (-value ^ *byte) & (1 << bit);
}

static void set_pixel_color(GBitmapDataRowInfo info, GPoint point, GColor color) {
#if defined(PBL_COLOR)
  // Write the pixel's byte color
  memset(&info.data[point.x], color.argb, 1);
#elif defined(PBL_BW)
  // Find the correct byte, then set the appropriate bit
  uint8_t byte = point.x / 8;
  uint8_t bit = point.x % 8;
  byte_set_bit(&info.data[byte], bit, gcolor_equal(color, GColorWhite) ? 1 : 0);
#endif
}

static void set_pixel_color_scaled(GBitmap *fb, GPoint origin, GPoint relative_pixel, int scale, GColor color) {
  int min_x = origin.x + relative_pixel.x * scale;
  int max_x = origin.x + relative_pixel.x * scale + scale;
  int min_y = origin.y + relative_pixel.y * scale;
  int max_y = origin.y + relative_pixel.y * scale + scale;

  for (int y = min_y; y < max_y; y++) {
    GBitmapDataRowInfo info = gbitmap_get_data_row_info(fb, y);
    for (int x = min_x; x < max_x; x++) {
      set_pixel_color(info, GPoint(x, y), color);
    }
  }
}

static void draw_filled_bar_top(GBitmap *fb, GPoint p, GColor color) {
    GBitmapDataRowInfo row_info = gbitmap_get_data_row_info(fb, p.y);
    set_pixel_color(row_info, GPoint(p.x, p.y), color);
    set_pixel_color(row_info, GPoint(p.x + 1, p.y), color);

    row_info = gbitmap_get_data_row_info(fb, p.y - 1);
    set_pixel_color(row_info, GPoint(p.x, p.y - 1), color);
    set_pixel_color(row_info, GPoint(p.x + 1, p.y - 1), color);
    set_pixel_color(row_info, GPoint(p.x - 1, p.y - 1), color);
    set_pixel_color(row_info, GPoint(p.x + 2, p.y - 1), color);

    row_info = gbitmap_get_data_row_info(fb, p.y - 2);
    set_pixel_color(row_info, GPoint(p.x, p.y - 2), color);
    set_pixel_color(row_info, GPoint(p.x + 1, p.y - 2), color);
    set_pixel_color(row_info, GPoint(p.x - 1, p.y - 2), color);
    set_pixel_color(row_info, GPoint(p.x + 2, p.y - 2), color);

    row_info = gbitmap_get_data_row_info(fb, p.y - 3);
    set_pixel_color(row_info, GPoint(p.x, p.y - 3), color);
    set_pixel_color(row_info, GPoint(p.x + 1, p.y - 3), color);
}

static void draw_bar_top(GBitmap *fb, GPoint p) {
  for (int y = p.y; y > p.y - 6; y--) {
    GBitmapDataRowInfo row_info = gbitmap_get_data_row_info(fb, y);
    int min_x = p.x - 3 + (p.y - y - 2);
    int max_x = p.x + 4 - (p.y - y - 2);

    if(y == p.y) {
      min_x = p.x;
      max_x = p.x + 1;
    } else if (y == p.y - 1) {
      min_x = p.x - 3;
      max_x = p.x + 4;
    }

    for (int x = min_x; x <= max_x; x++) {
      GColor color = GColorBlack;
      if (y != p.y && (x == min_x || x == max_x)) {
        color = GColorWhite;
      }

      set_pixel_color(row_info, GPoint(x, y), color);
    }
  }
}

static void draw_bar(GBitmap *fb, GPoint p, int segments, int fill, enum BarColor bar_color) {
  fill = MIN(fill, segments * 8);

  GColor left_color;
  GColor right_color;
  GColor top_color;
  switch (bar_color) {
    case RED:
      left_color = GColorFashionMagenta;
      right_color = GColorRichBrilliantLavender;
      top_color = GColorMelon;
      break;
    case GREEN:
      left_color = GColorGreen;
      right_color = GColorMediumSpringGreen;
      top_color = GColorMintGreen;
      break;
    case BLUE:
      left_color = GColorBlue;
      right_color = GColorVeryLightBlue;
      top_color = GColorBabyBlueEyes;
      break;
    case BW:
    default:
      left_color = GColorWhite;
      right_color = GColorWhite;
      top_color = GColorWhite;
      break;
  }

  for (int s = 0; s < segments; s++) {
    int y = p.y - s * 10;

    for (int h = 0; h < 10; h++) {
      y = y - 1;
      GBitmapDataRowInfo row_info = gbitmap_get_data_row_info(fb, y);
      GBitmapDataRowInfo upper_row_info = gbitmap_get_data_row_info(fb, y-1);

      if (h == 0) {

//set_pixel_color_scaled(GBitmap *fb, GPoint origin, GPoint relative_pixel, int scale, GColor color) {
        set_pixel_color_scaled(fb, p, GPoint(0, -h-s*10-1), 2, GColorWhite);
        set_pixel_color_scaled(fb, p, GPoint(1, -h-s*10-1), 2, GColorWhite);
        set_pixel_color_scaled(fb, p, GPoint(-1, -h-s*10-2), 2, GColorWhite);
        set_pixel_color_scaled(fb, p, GPoint(2, -h-s*10-2), 2, GColorWhite);

        if (s != 0) {
          set_pixel_color_scaled(fb, p, GPoint(-2, -h-s*10-2), 2, GColorBlack);
          set_pixel_color_scaled(fb, p, GPoint(3, -h-s*10-2), 2, GColorBlack);
          set_pixel_color_scaled(fb, p, GPoint(-3, -h-s*10-2), 2, GColorWhite);
          set_pixel_color_scaled(fb, p, GPoint(4, -h-s*10-2), 2, GColorWhite);
        }
      } else if (h == 1) {
        set_pixel_color_scaled(fb, p, GPoint(0, -h-s*10-1), 2, GColorBlack);
        set_pixel_color_scaled(fb, p, GPoint(1, -h-s*10-1), 2, GColorBlack);
        set_pixel_color_scaled(fb, p, GPoint(-1, -h-s*10-2), 2, GColorBlack);
        set_pixel_color_scaled(fb, p, GPoint(2, -h-s*10-2), 2, GColorBlack);
        set_pixel_color_scaled(fb, p, GPoint(-2, -h-s*10-2), 2, GColorWhite);
        set_pixel_color_scaled(fb, p, GPoint(3, -h-s*10-2), 2, GColorWhite);

        if (s != 0) {
          set_pixel_color_scaled(fb, p, GPoint(-3, -h-s*10-2), 2, GColorWhite);
          set_pixel_color_scaled(fb, p, GPoint(4, -h-s*10-2), 2, GColorWhite);
        }
      } else {
        if (fill > s * 8 + h - 2) {
          set_pixel_color_scaled(fb, p, GPoint(0, -h-s*10-1), 2, left_color);
          set_pixel_color_scaled(fb, p, GPoint(1, -h-s*10-1), 2, right_color);
          set_pixel_color_scaled(fb, p, GPoint(-1, -h-s*10-2), 2, left_color);
          set_pixel_color_scaled(fb, p, GPoint(2, -h-s*10-2), 2, right_color);
        }
        else {
          set_pixel_color_scaled(fb, p, GPoint(0, -h-s*10-1), 2, GColorBlack);
          set_pixel_color_scaled(fb, p, GPoint(1, -h-s*10-1), 2, GColorBlack);
          set_pixel_color_scaled(fb, p, GPoint(-1, -h-s*10-2), 2, GColorBlack);
          set_pixel_color_scaled(fb, p, GPoint(2, -h-s*10-2), 2, GColorBlack);
        }
        set_pixel_color_scaled(fb, p, GPoint(-2, -h-s*10-2), 2, GColorBlack);
        set_pixel_color_scaled(fb, p, GPoint(3, -h-s*10-2), 2, GColorBlack);
        set_pixel_color_scaled(fb, p, GPoint(-3, -h-s*10-2), 2, GColorWhite);
        set_pixel_color_scaled(fb, p, GPoint(4, -h-s*10-2), 2, GColorWhite);
      }
    }
  }
  draw_bar_top(fb, GPoint(p.x, p.y - segments * 10));

  if ((fill / 8) == segments) {
    draw_filled_bar_top(fb, GPoint(p.x, p.y - ((fill / 8) * 10) - (fill % 8)), top_color);
  } else if (fill == 0) {
    draw_filled_bar_top(fb, GPoint(p.x, p.y - 3), top_color);
  } else if (fill % 8 == 0) {
    draw_filled_bar_top(fb, GPoint(p.x, p.y - ((fill / 8) * 10 + 2) - 1), top_color);
  } else {
    draw_filled_bar_top(fb, GPoint(p.x, p.y - ((fill / 8) * 10 + 2) - (fill % 8)), top_color);
  }
}


static void bars_update_proc(Layer *layer, GContext *ctx) {
  GBitmap *fb = graphics_capture_frame_buffer(ctx);

  draw_bar(fb, GPoint(15, 95), 3, 18, RED);
  draw_bar(fb, GPoint(35, 95), 2, 13, GREEN);
  draw_bar(fb, GPoint(55, 95), 1, 8, BLUE);

  graphics_release_frame_buffer(ctx, fb);
}

static void update_time() {
  static char buffer[8];
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

#ifdef HOUR
  t->tm_hour = HOUR;
#endif
#ifdef MINUTE
  t->tm_min = MINUTE;
#endif

  if (clock_is_24h_style()) {
    strftime(buffer, sizeof(buffer), "%H:%M", t);
  } else {
    strftime(buffer, sizeof(buffer), "%-I:%M %p", t);
  }
  text_layer_set_text(time_shadow_layer, buffer);
  text_layer_set_text(time_layer, buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void background_update_proc(Layer *layer, GContext *ctx) {
#ifdef PBL_COLOR
  graphics_context_set_fill_color(ctx, GColorDarkGreen);
#else
  graphics_context_set_fill_color(ctx, GColorLightGray);
#endif
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  background_layer = layer_create(bounds);
  layer_set_update_proc(background_layer, background_update_proc);
  layer_add_child(window_layer, background_layer);

#ifdef PBL_PLATFORM_EMERY
  font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ODIN_ROUNDED_BOLD_45));
#else
  font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_ODIN_ROUNDED_BOLD_33));
#endif

  fox_bitmap = gbitmap_create_with_resource(RESOURCE_ID_FOX);
  GSize fox_size = gbitmap_get_bounds(fox_bitmap).size;
#if defined(PBL_PLATFORM_CHALK)
  fox_layer = bitmap_layer_create(GRect((bounds.size.w - fox_size.w) / 2, 3, fox_size.w, fox_size.h));
#elif defined(PBL_PLATFORM_EMERY)
  fox_layer = bitmap_layer_create(GRect((bounds.size.w - fox_size.w) / 2, 8, fox_size.w, fox_size.h));
#else
  fox_layer = bitmap_layer_create(GRect((bounds.size.w - fox_size.w) / 2, 6, fox_size.w, fox_size.h));
#endif
  bitmap_layer_set_compositing_mode(fox_layer, GCompOpSet);
  bitmap_layer_set_bitmap(fox_layer, fox_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(fox_layer));

  border_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BORDER);
  GSize border_size = gbitmap_get_bounds(border_bitmap).size;
#if defined(PBL_PLATFORM_CHALK)
  border_layer = bitmap_layer_create(GRect((bounds.size.w - border_size.w) / 2, 94, border_size.w, border_size.h));
#elif defined(PBL_PLATFORM_EMERY)
  border_layer = bitmap_layer_create(GRect((bounds.size.w - border_size.w) / 2, bounds.size.h - border_size.h - 8, border_size.w, border_size.h));
#else
  border_layer = bitmap_layer_create(GRect((bounds.size.w - border_size.w) / 2, bounds.size.h - border_size.h - 6, border_size.w, border_size.h));
#endif
  bitmap_layer_set_compositing_mode(border_layer, GCompOpSet);
  bitmap_layer_set_bitmap(border_layer, border_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(border_layer));

#if defined(PBL_PLATFORM_CHALK)
  time_shadow_layer = text_layer_create(GRect(0, 104, bounds.size.w, 156));
#elif defined(PBL_PLATFORM_EMERY)
  time_shadow_layer = text_layer_create(GRect(0, 149, bounds.size.w, 202));
#else
  time_shadow_layer = text_layer_create(GRect(0, 112, bounds.size.w, 165));
#endif
  text_layer_set_background_color(time_shadow_layer, GColorClear);
#ifdef PBL_COLOR
  text_layer_set_text_color(time_shadow_layer,GColorBulgarianRose);
#else
  text_layer_set_text_color(time_shadow_layer,GColorBlack);
#endif
  text_layer_set_font(time_shadow_layer, font);
  text_layer_set_text_alignment(time_shadow_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_shadow_layer));

#if defined(PBL_PLATFORM_CHALK)
  time_layer = text_layer_create(GRect(0, 102, bounds.size.w, 154));
#elif defined(PBL_PLATFORM_EMERY)
  time_layer = text_layer_create(GRect(0, 147, bounds.size.w, 200));
#else
  time_layer = text_layer_create(GRect(0, 110, bounds.size.w, 163));
#endif
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_font(time_layer, font);
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(time_layer));

  bars_layer = layer_create(bounds);
  layer_set_update_proc(bars_layer, bars_update_proc);
  layer_add_child(window_layer, bars_layer);

  update_time();

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void window_unload(Window *window) {
  layer_destroy(background_layer);

  gbitmap_destroy(fox_bitmap);
  bitmap_layer_destroy(fox_layer);

  gbitmap_destroy(border_bitmap);
  bitmap_layer_destroy(border_layer);

  text_layer_destroy(time_layer);
  text_layer_destroy(time_shadow_layer);
  fonts_unload_custom_font(font);

  layer_destroy(bars_layer);
}

static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
