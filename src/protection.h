#ifndef __PROTECTION_H__
#define __PROTECTION_H__

#include <stdint.h>

#define PROTECTION_STATUS_OV (1 << 0)
#define PROTECTION_STATUS_UV (1 << 1)
#define PROTECTION_STATUS_OT (1 << 2)
#define PROTECTION_STATUS_FAULT (1 << 7)

class Protection {
public:
  Protection(Measurements *measurements);
  void update();
  void fault();
  uint8_t status();

private:
  Measurements *_measurements;
  bool _ov;
  bool _uv;
  bool _fault;
};

#endif
