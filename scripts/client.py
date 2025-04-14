# pip install pymodbus

import argparse
import json
import sys
import time
from pymodbus.client import ModbusSerialClient
from pymodbus import FramerType

class Modbus:

    def __init__(self, device: str, baudrate=115200):
        self.client = ModbusSerialClient(
            port=device,
            framer=FramerType.RTU,
            baudrate=baudrate,
            timeout=10,
        )
        

    def connect(self):
        self.client.connect()

    def read_registers(self, address: int, count: int):
        res = self.client.read_holding_registers(address=address, count=count)
        if not hasattr(res, "registers"):
            raise Exception(res)
        return res.registers
    
def temp(msb: int, lsb: int) -> float:
    integral = ((msb << 8) | lsb) >> 4
    decimal = lsb & 0xF

    sign = msb & 0b10000000
    if sign:
        integral *= -1

    temp_c = integral + (decimal * 0.0625)
    temp_f = temp_c * 9 / 5 + 32

    return temp_f

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--device", help="serial device")
    parser.add_argument("--interval", help="polling interval (in milliseconds)", default=3000, type=int)
    args = parser.parse_args()

    modbus = Modbus(args.device)
    modbus.connect()
    time.sleep(1)

    while True:
        res = modbus.read_registers(address=1, count=7)
        data = dict(
            version=res[0],
            connected=res[1],
            error_code=res[2],
            num_cells=res[3],
            round_trip_time=res[4],
            temp=temp(res[5], res[6]),
        )

        if data["num_cells"] > 0:
            res = modbus.read_registers(address=8, count=data["num_cells"])
            data["cell_voltages"] = res
        print(json.dumps(data, indent=4))
        sys.stdout.flush()
        time.sleep(args.interval / 1000)

if __name__ == "__main__":
    main()
