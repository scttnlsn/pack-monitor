import argparse
import datetime
import json
import logging
import sys
import time
from pymodbus.client.sync import ModbusSerialClient

def int32(values):
    msb, lsb = values
    return (msb << 16) | lsb

class Measurements(object):

    def __init__(self, modbus, num_cells):
        self.modbus = modbus
        self.num_cells = num_cells

    def sample(self):
        registers = self.read_registers()
        return {
            'timestamp': datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            'status': registers[0],
            'charge_current': int32(registers[1:3]),
            'discharge_current': int32(registers[3:5]),
            'cc_charge': int32(registers[5:7]),
            'cc_discharge': int32(registers[7:9]),
            'cc_net': int32(registers[9:11]),
            'cc_counter': int32(registers[11:13]),
            'soc': int32(registers[13:15]),
            'cell_voltages': registers[15:]
        }

    def read_registers(self):
        count = 15 + self.num_cells
        res = self.modbus.read_input_registers(address=0, count=count, unit=1)

        if not hasattr(res, 'registers'):
            raise res

        return res.registers

def connect(port):
    modbus = ModbusSerialClient(method='rtu', port=port, baudrate=115200)

    res = modbus.connect()
    if not res:
        raise 'error connecting to serial client'

    return modbus

def main():
    parser = argparse.ArgumentParser(description='communicate with charge controller')
    parser.add_argument('--port', help='serial device name')
    parser.add_argument('--interval', help='polling interval (in milliseconds)', default=1000, type=int)
    parser.add_argument('--num-cells', help='the number of cells', default=8, type=int)
    parser.add_argument('--debug', help='show debugging messages')
    args = parser.parse_args()

    modbus = connect(args.port)
    measurements = Measurements(modbus, args.num_cells)

    if args.debug:
        logging.basicConfig()
        log = logging.getLogger()
        log.setLevel(logging.DEBUG)

    while True:
        print(json.dumps(measurements.sample()))
        sys.stdout.flush()
        time.sleep(args.interval / 1000)

if __name__ == '__main__':
    main()
