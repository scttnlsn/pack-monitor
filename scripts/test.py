import logging
import time

FORMAT = ('%(asctime)-15s %(threadName)-15s '
          '%(levelname)-8s %(module)-15s:%(lineno)-8s %(message)s')
logging.basicConfig(format=FORMAT)
log = logging.getLogger()
log.setLevel(logging.DEBUG)

from pymodbus.client.sync import ModbusSerialClient
modbus = ModbusSerialClient(method='rtu', port='/dev/ttyACM0', baudrate=115200)
print('connecting...')
modbus.connect()
time.sleep(1)
print('connected.')

res = modbus.read_holding_registers(address=1, count=2, unit=1)
print(res.registers)

modbus.write_register(1, 1, unit=1)
modbus.write_register(2, 1, unit=1)

res = modbus.read_holding_registers(address=1, count=2, unit=1)
print(res.registers)
