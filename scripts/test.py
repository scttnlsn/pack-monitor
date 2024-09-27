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

fixed_registers = 6
num_cells = 2
max_cells = 128

# calibrate voltage references:
# new_ref_voltage = measured_voltage * current_ref_voltage / current_reported_voltage

reg_voltage_ref = fixed_registers + max_cells + 1
# modbus.write_register(address=reg_voltage_ref + 0, value=1082)
# modbus.write_register(address=reg_voltage_ref + 1, value=1102)

reg_cell_voltages = fixed_registers + 1

# res = modbus.read_holding_registers(address=fixed_registers + 1, count=num_cells)
res = modbus.read_holding_registers(address=7, count=2)
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
