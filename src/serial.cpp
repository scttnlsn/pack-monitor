#include <Arduino.h>
#include <stdarg.h>

#include "serial.h"

namespace serial {
  FILE serial_stdout;

  int serial_putchar(char c, FILE* f) {
    if (c == '\n') {
      serial_putchar('\r', f);
    }

    return Serial.write(c) == 1 ? 0 : 1;
  }

  void init(uint32_t baud) {
    Serial.begin(baud);

    fdev_setup_stream(&serial_stdout, serial_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &serial_stdout;
  }

  void log(const char *level, const char *module, const char* format, ...) {
    va_list args;
    fprintf(stdout, "log/%s/%s/", level, module);
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
    fprintf(stdout, "\n");
  }

  void value(const char *name, char *value) {
    printf("value/%s/%s\n", name, value);
  }
}
