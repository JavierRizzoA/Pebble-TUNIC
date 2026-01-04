#include <pebble.h>

static Window *window;
static Layer *background_layer;
static GFont font;
static BitmapLayer *fox_layer;
static GBitmap *fox_bitmap;
static BitmapLayer *border_layer;
static GBitmap *border_bitmap;
static TextLayer *time_shadow_layer;
static TextLayer *time_layer;

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
