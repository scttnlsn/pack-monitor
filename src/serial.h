#ifndef __SERIAL_H__
#define __SERIAL_H__

namespace serial {
  void init();
  void log(const char *level, const char *module, const char *format, ...);
  void value(const char *name, char *value);
}

#endif
