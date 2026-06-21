#include "events.h"
#include "../INTERRUPTS/kb.h"

static struct input_event events[INPUT_EVENT_QUEUE_SIZE];
static unsigned int event_head = 0;
static unsigned int event_tail = 0;
static unsigned int event_count = 0;

void input_events_init(void) {
  event_head = 0;
  event_tail = 0;
  event_count = 0;
}

int input_push_event(const struct input_event *event) {
  if(!event) return -1;
  if(event_count >= INPUT_EVENT_QUEUE_SIZE) return -1;

  events[event_tail] = *event;
  event_tail = (event_tail + 1) % INPUT_EVENT_QUEUE_SIZE;
  event_count++;
  return 0;
}

int input_pop_event(struct input_event *event_out) {
  if(!event_out) return -1;
  if(event_count == 0) return 0;

  *event_out = events[event_head];
  event_head = (event_head + 1) % INPUT_EVENT_QUEUE_SIZE;
  event_count--;
  return 1;
}

void input_poll_keyboard(void) {
  struct input_event event;
  unsigned char scan_code;

  if(!keyboard_has_data()) return;

  scan_code = keyboard_read_scan_code();
  if(scan_code & 0x80) return;

  event.type = INPUT_EVENT_KEY;
  event.code = scan_code;
  event.x = 0;
  event.y = 0;
  event.pressed = 1;
  event.ascii = 0;

  if(scan_code <= KEYBOARD_MAX_ASCII) {
    event.ascii = (char)keyboard_scan_code_to_ascii(scan_code);
  }

  input_push_event(&event);
}

void input_push_mouse_move(int dx, int dy) {
  struct input_event event;
  event.type = INPUT_EVENT_MOUSE_MOVE;
  event.code = 0;
  event.x = dx;
  event.y = dy;
  event.pressed = 0;
  event.ascii = 0;
  input_push_event(&event);
}

void input_push_mouse_button(unsigned int button, int pressed) {
  struct input_event event;
  event.type = INPUT_EVENT_MOUSE_BUTTON;
  event.code = button;
  event.x = 0;
  event.y = 0;
  event.pressed = pressed;
  event.ascii = 0;
  input_push_event(&event);
}
