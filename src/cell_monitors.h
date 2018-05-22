#ifndef __CELL_MONITORS_H__
#define __CELL_MONITORS_H__

#include <stdint.h>
#include <Stream.h>

typedef struct {
  uint8_t address;
  uint8_t request;
  uint8_t reg;
  uint8_t write;
  uint16_t value;
} packet_t;

class CellMonitors {
 public:
  CellMonitors(Stream &stream);
  int begin();

 private:
  Stream &_stream;
  uint8_t _num_cells;
  int send(packet_t *packet);
  int receive(packet_t *packet);
};

#endif
