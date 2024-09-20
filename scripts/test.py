# pip install pyserial pymodbus

# import logging
# logging.basicConfig()
# log = logging.getLogger()
# log.setLevel(logging.DEBUG)

from pymodbus.client import ModbusSerialClient
from pymodbus import FramerType

modbus = ModbusSerialClient(
    port="/dev/ttyACM0",
    framer=FramerType.RTU,
    baudrate=115200,
)
modbus.connect()

res = modbus.read_holding_registers(address=1, count=4)
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
