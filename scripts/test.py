import logging
import time

# logging.basicConfig()
# log = logging.getLogger()
# log.setLevel(logging.DEBUG)

from pymodbus.client.sync import ModbusSerialClient
modbus = ModbusSerialClient(method='rtu', port='/dev/ttyACM0', baudrate=115200)
modbus.connect()

res = modbus.read_holding_registers(address=1, count=2, unit=1)
print(res.registers)

# modbus.write_register(1, 1, unit=1)
# modbus.write_register(2, 1, unit=1)

# res = modbus.read_holding_registers(address=1, count=2, unit=1)
# print(res.registers)

temp_msb = res.registers[0]
temp_lsb = res.registers[1]

integral = ((temp_msb << 8) | temp_lsb) >> 4
decimal = temp_lsb & 0xF

sign = temp_msb & 0b10000000;
if sign > 0:
    integral *= -1

temp_c = integral + (decimal * 0.0625)
temp_f = temp_c * 9 / 5 + 32

print(temp_f)
