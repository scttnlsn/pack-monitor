import argparse
import json
import sys
import time
from pymodbus.client.sync import ModbusSerialClient

class Modbus:

    def __init__(self, device, baudrate=115200):
        self.client = ModbusSerialClient(method='rtu', port=device, baudrate=baudrate)

    def connect(self):
        res = self.client.connect()
        if not res:
            raise f'error connecting to serial device: {device}'


    def read_registers(self):
        res = self.client.read_holding_registers(address=1, count=2, unit=1)

        if not hasattr(res, 'registers'):
            raise Exception(res)

        return res.registers

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--device', help='serial device')
    parser.add_argument('--interval', help='polling interval (in milliseconds)', default=1000, type=int)
    args = parser.parse_args()

    modbus = Modbus(args.device)
    modbus.connect()
    time.sleep(1)

    while True:
        data = modbus.read_registers()
        print(json.dumps(data))
        sys.stdout.flush()
        time.sleep(args.interval / 1000)

if __name__ == '__main__':
    main()
