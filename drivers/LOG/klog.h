#ifndef INCLUDE_KLOG_H
#define INCLUDE_KLOG_H

#define KLOG_SINK_SERIAL 0x1
#define KLOG_SINK_DISPLAY 0x2
#define KLOG_SINK_BOTH (KLOG_SINK_SERIAL | KLOG_SINK_DISPLAY)

void klog_init(unsigned int sinks);
void klog_enable_sink(unsigned int sink);
void klog_disable_sink(unsigned int sink);
void klog_write(const char *buffer, unsigned int length);
void klog_write_string(const char *string);

#endif
