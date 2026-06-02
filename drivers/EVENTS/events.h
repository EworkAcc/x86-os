#ifndef INCLUDE_EVENTS_H
#define INCLUDE_EVENTS_H

#define INPUT_EVENT_QUEUE_SIZE 32

#define INPUT_EVENT_NONE 0
#define INPUT_EVENT_KEY 1
#define INPUT_EVENT_MOUSE_MOVE 2
#define INPUT_EVENT_MOUSE_BUTTON 3

#define INPUT_MOUSE_LEFT 1

struct input_event {
  unsigned int type;
  unsigned int code;
  int x;
  int y;
  int pressed;
  char ascii;
};

void input_events_init(void);
int input_push_event(const struct input_event *event);
int input_pop_event(struct input_event *event_out);
void input_poll_keyboard(void);
void input_push_mouse_move(int dx, int dy);
void input_push_mouse_button(unsigned int button, int pressed);

#endif
