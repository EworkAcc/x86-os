#include "klog.h"
#include "../DISPLAY/display.h"
#include "../SP/sp.h"

static unsigned int active_sinks = KLOG_SINK_SERIAL;

void klog_init(unsigned int sinks) {
  active_sinks = sinks;
}

void klog_enable_sink(unsigned int sink) {
  active_sinks |= sink;
}

void klog_disable_sink(unsigned int sink) {
  active_sinks &= ~sink;
}

void klog_write(const char *buffer, unsigned int length) {
  if(active_sinks & KLOG_SINK_SERIAL) {
    serial_write((char *)buffer, length);
  }

  if(active_sinks & KLOG_SINK_DISPLAY) {
    display_write(buffer, length);
  }
}

void klog_write_string(const char *string) {
  unsigned int length = 0;
  while(string[length] != 0) length++;
  klog_write(string, length);
}
