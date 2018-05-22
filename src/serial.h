#ifndef __SERIAL_H__
#define __SERIAL_H__

namespace serial {
  void init();
  void error(const char *format, ...);
  void value(const char *name, char *value);
}

#endif
