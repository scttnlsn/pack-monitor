# pip install pyserial pymodbus

import logging
logging.basicConfig()
log = logging.getLogger()
log.setLevel(logging.DEBUG)

from pymodbus.client import ModbusSerialClient
from pymodbus import FramerType
import time

modbus = ModbusSerialClient(
    port="/dev/ttyACM0",
    framer=FramerType.RTU,
    baudrate=115200,
    timeout=10,
)
modbus.connect()

num_cells = 2
max_cells = 128
fixed_registers = 7
reg_cell_voltages = fixed_registers + 1
reg_voltage_ref = fixed_registers + max_cells + 1

# calibrate voltage references:
# new_ref_voltage = measured_voltage * current_ref_voltage / current_reported_voltage

# modbus.write_register(address=reg_voltage_ref + 0, value=1082)
# modbus.write_register(address=reg_voltage_ref + 1, value=1102)

res = modbus.read_holding_registers(address=reg_cell_voltages, count=num_cells)
print(res)
print(res.registers)

# temp_msb = res.registers[0]
# temp_lsb = res.registers[1]

# integral = ((temp_msb << 8) | temp_lsb) >> 4
# decimal = temp_lsb & 0xF

# sign = temp_msb & 0b10000000;
# if sign > 0:
#     integral *= -1

# temp_c = integral + (decimal * 0.0625)
# temp_f = temp_c * 9 / 5 + 32

# print(temp_f)
