# pip install pymodbus

import argparse
import json
import sys
import time
from typing import Any, Dict, List

from pymodbus import FramerType
from pymodbus.client import ModbusSerialClient


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

    def write_register(self, address: int, value: int):
        res = self.client.write_register(address, value)
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


class Flags:

    def __init__(self, flags: List[str]):
        self.flags = flags

    def __call__(self, value: int) -> Dict[str, bool]:
        return {
            flag: bool(value & (1 << shift)) for shift, flag in enumerate(self.flags)
        }


status = Flags(["connecting", "connected"])
protection = Flags(["ov", "uv", "ut", "fault"])
error = Flags(["fault", "watchdog", "crc", "timeout", "no_response", "modbus"])


def get_state(modbus: Modbus) -> Dict[str, Any]:
    res = modbus.read_registers(address=1, count=8)
    data = dict(
        version=res[0],
        status=status(res[1]),
        protection=protection(res[2]),
        errors=error(res[3]),
        num_cells=res[4],
        round_trip_time=res[5],
        temp=temp(res[6], res[7]),
    )

    if data["num_cells"] > 0:
        res = modbus.read_registers(address=9, count=data["num_cells"])
        data["cell_voltages"] = res

    return data


def write_cell_voltage(modbus: Modbus, index: int, voltage: int):
    return modbus.write_register(address=9 + index, value=voltage)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--device", help="serial device")
    parser.add_argument(
        "--interval", help="polling interval (in milliseconds)", default=3000, type=int
    )
    args = parser.parse_args()

    modbus = Modbus(args.device)
    modbus.connect()
    time.sleep(1)

    while True:
        data = get_state(modbus)
        print(json.dumps(data, indent=4))
        sys.stdout.flush()
        time.sleep(args.interval / 1000)


if __name__ == "__main__":
    main()
