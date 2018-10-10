#ifndef __SERIAL_H__
#define __SERIAL_H__

#include <stdint.h>

namespace serial {
  void init(uint32_t baud);
  void log(const char *level, const char *module, const char *format, ...);
  void value(const char *name, char *value);
}

#endif
