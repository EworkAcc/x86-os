#include "desktop.h"
#include "../EVENTS/events.h"
#include "../GRAPHICS/gfx.h"
#include "../LOG/klog.h"

#define DESKTOP_MAX_WINDOWS 4
#define DESKTOP_MAX_BUTTONS 8
#define DESKTOP_TITLE_HEIGHT 18
#define DESKTOP_BUTTON_HEIGHT 18
#define DESKTOP_MOUSE_STEP 8

struct desktop_window {
  int used;
  int x;
  int y;
  unsigned int width;
  unsigned int height;
  unsigned int title_color;
  const char *title;
  const char *body;
};

struct desktop_button {
  int used;
  int window;
  int x;
  int y;
  unsigned int width;
  unsigned int height;
  const char *label;
  unsigned int toggled;
};

static struct desktop_window windows[DESKTOP_MAX_WINDOWS];
static struct desktop_button buttons[DESKTOP_MAX_BUTTONS];
static int mouse_x = 40;
static int mouse_y = 40;
static unsigned int repaint_requested = 1;
static unsigned int click_count = 0;

static void desktop_request_repaint(void) {
  repaint_requested = 1;
}

static int desktop_add_window(int x, int y, unsigned int width, unsigned int height, const char *title, const char *body) {
  unsigned int i;
  for(i = 0; i < DESKTOP_MAX_WINDOWS; i++) {
    if(!windows[i].used) {
      windows[i].used = 1;
      windows[i].x = x;
      windows[i].y = y;
      windows[i].width = width;
      windows[i].height = height;
      windows[i].title_color = GFX_COLOR_BLUE;
      windows[i].title = title;
      windows[i].body = body;
      return (int)i;
    }
  }
  return -1;
}

static int desktop_add_button(int window, int x, int y, unsigned int width, const char *label) {
  unsigned int i;
  for(i = 0; i < DESKTOP_MAX_BUTTONS; i++) {
    if(!buttons[i].used) {
      buttons[i].used = 1;
      buttons[i].window = window;
      buttons[i].x = x;
      buttons[i].y = y;
      buttons[i].width = width;
      buttons[i].height = DESKTOP_BUTTON_HEIGHT;
      buttons[i].label = label;
      buttons[i].toggled = 0;
      return (int)i;
    }
  }
  return -1;
}

static void desktop_draw_frame(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned int color) {
  gfx_draw_rect(x, y, width, 1, color);
  gfx_draw_rect(x, y + height - 1, width, 1, color);
  gfx_draw_rect(x, y, 1, height, color);
  gfx_draw_rect(x + width - 1, y, 1, height, color);
}

static void desktop_draw_button(const struct desktop_button *button) {
  const struct desktop_window *window;
  unsigned int x;
  unsigned int y;
  unsigned int fill;

  if(!button || !button->used) return;
  if(button->window < 0 || button->window >= DESKTOP_MAX_WINDOWS) return;

  window = &windows[button->window];
  if(!window->used) return;

  x = (unsigned int)(window->x + button->x);
  y = (unsigned int)(window->y + button->y);
  fill = button->toggled ? GFX_COLOR_GREEN : GFX_COLOR_GREY;

  gfx_draw_rect(x, y, button->width, button->height, fill);
  desktop_draw_frame(x, y, button->width, button->height, GFX_COLOR_WHITE);
  gfx_draw_string(x + 6, y + 5, button->label, GFX_COLOR_BLACK, fill);
}

static void desktop_draw_window(const struct desktop_window *window) {
  unsigned int i;

  if(!window || !window->used) return;

  gfx_draw_rect((unsigned int)window->x, (unsigned int)window->y, window->width, window->height, GFX_COLOR_GREY);
  gfx_draw_rect((unsigned int)window->x, (unsigned int)window->y, window->width, DESKTOP_TITLE_HEIGHT, window->title_color);
  desktop_draw_frame((unsigned int)window->x, (unsigned int)window->y, window->width, window->height, GFX_COLOR_WHITE);
  gfx_draw_string((unsigned int)window->x + 6, (unsigned int)window->y + 5, window->title, GFX_COLOR_WHITE, window->title_color);
  gfx_draw_string((unsigned int)window->x + 8, (unsigned int)window->y + 28, window->body, GFX_COLOR_BLACK, GFX_COLOR_GREY);

  for(i = 0; i < DESKTOP_MAX_BUTTONS; i++) {
    if(buttons[i].used && buttons[i].window >= 0 && &windows[buttons[i].window] == window) desktop_draw_button(&buttons[i]);
  }
}

static void desktop_draw_cursor(void) {
  unsigned int x = (unsigned int)mouse_x;
  unsigned int y = (unsigned int)mouse_y;
  gfx_draw_rect(x, y, 2, 16, GFX_COLOR_WHITE);
  gfx_draw_rect(x, y, 12, 2, GFX_COLOR_WHITE);
  gfx_draw_rect(x + 2, y + 2, 2, 10, GFX_COLOR_BLACK);
  gfx_draw_rect(x + 4, y + 4, 2, 8, GFX_COLOR_BLACK);
}

static int desktop_button_contains(const struct desktop_button *button, int x, int y) {
  const struct desktop_window *window;
  int left;
  int top;

  if(!button || !button->used) return 0;
  if(button->window < 0 || button->window >= DESKTOP_MAX_WINDOWS) return 0;
  window = &windows[button->window];
  if(!window->used) return 0;

  left = window->x + button->x;
  top = window->y + button->y;
  return x >= left && y >= top && x < left + (int)button->width && y < top + (int)button->height;
}

static void desktop_handle_click(void) {
  unsigned int i;
  for(i = 0; i < DESKTOP_MAX_BUTTONS; i++) {
    if(desktop_button_contains(&buttons[i], mouse_x, mouse_y)) {
      buttons[i].toggled = !buttons[i].toggled;
      if(buttons[i].window >= 0 && buttons[i].window < DESKTOP_MAX_WINDOWS) {
        windows[buttons[i].window].title_color = buttons[i].toggled ? GFX_COLOR_GREEN : GFX_COLOR_BLUE;
      }
      click_count++;
      desktop_request_repaint();
      return;
    }
  }
}

static void desktop_clamp_mouse(void) {
  const struct gfx_state *state = gfx_get_state();
  if(mouse_x < 0) mouse_x = 0;
  if(mouse_y < 0) mouse_y = 0;
  if(state->width > 0 && mouse_x > (int)state->width - 16) mouse_x = (int)state->width - 16;
  if(state->height > 0 && mouse_y > (int)state->height - 16) mouse_y = (int)state->height - 16;
}

static int desktop_handle_event(const struct input_event *event) {
  if(!event) return 1;

  if(event->type == INPUT_EVENT_MOUSE_MOVE) {
    mouse_x += event->x;
    mouse_y += event->y;
    desktop_clamp_mouse();
    desktop_request_repaint();
  } else if(event->type == INPUT_EVENT_MOUSE_BUTTON && event->pressed) {
    desktop_handle_click();
  } else if(event->type == INPUT_EVENT_KEY) {
    if(event->ascii == 'q') return 0;
    if(event->ascii == 'w') input_push_mouse_move(0, -DESKTOP_MOUSE_STEP);
    else if(event->ascii == 's') input_push_mouse_move(0, DESKTOP_MOUSE_STEP);
    else if(event->ascii == 'a') input_push_mouse_move(-DESKTOP_MOUSE_STEP, 0);
    else if(event->ascii == 'd') input_push_mouse_move(DESKTOP_MOUSE_STEP, 0);
    else if(event->ascii == ' ' || event->ascii == '\n') input_push_mouse_button(INPUT_MOUSE_LEFT, 1);
  }

  return 1;
}

void desktop_repaint(void) {
  unsigned int i;
  char clicks[] = "clicks: 000000";
  unsigned int value = click_count;
  int pos = 13;

  if(!gfx_is_available()) return;

  while(pos >= 8) {
    clicks[pos] = (char)('0' + (value % 10));
    value /= 10;
    pos--;
  }

  gfx_draw_rect(0, 0, gfx_get_state()->width, gfx_get_state()->height, 0x202040);
  gfx_draw_string(8, 8, "x86-os desktop: wasd move, space/enter click, q console", GFX_COLOR_WHITE, 0x202040);
  gfx_draw_string(8, 22, clicks, GFX_COLOR_CYAN, 0x202040);

  for(i = 0; i < DESKTOP_MAX_WINDOWS; i++) desktop_draw_window(&windows[i]);
  desktop_draw_cursor();
  repaint_requested = 0;
}

void desktop_init(void) {
  unsigned int i;

  for(i = 0; i < DESKTOP_MAX_WINDOWS; i++) windows[i].used = 0;
  for(i = 0; i < DESKTOP_MAX_BUTTONS; i++) buttons[i].used = 0;

  mouse_x = 48;
  mouse_y = 48;
  click_count = 0;

  {
    int main_window = desktop_add_window(48, 58, 260, 130, "Desktop Window", "Tiny compositor + button");
    int info_window = desktop_add_window(340, 90, 250, 100, "Input Model", "Events drive repaint");
    if(main_window >= 0) desktop_add_button(main_window, 16, 78, 110, "Toggle");
    if(info_window >= 0) desktop_add_button(info_window, 16, 58, 120, "Click me");
  }

  desktop_request_repaint();
}

void desktop_run(void) {
  struct input_event event;

  if(!gfx_is_available()) {
    klog_write_string("desktop: RGB pixel framebuffer unavailable; staying in text console\n");
    return;
  }

  input_events_init();
  desktop_init();
  desktop_repaint();

  for(;;) {
    input_poll_keyboard();
    while(input_pop_event(&event) > 0) {
      if(!desktop_handle_event(&event)) {
        klog_write_string("desktop: returned to text console\n");
        return;
      }
    }

    if(repaint_requested) desktop_repaint();
  }
}
