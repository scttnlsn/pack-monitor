# pip install pyserial pymodbus

import logging
import time

from client import Modbus, get_state, write_cell_voltage

# logging.basicConfig()
# log = logging.getLogger()
# log.setLevel(logging.DEBUG)



modbus = Modbus(device="/dev/ttyACM0")
modbus.connect()
time.sleep(1)

print("connecting...")
while True:
    data = get_state(modbus)
    if data["status"]["connected"]:
        break

num_cells = data["num_cells"]
print(f"{num_cells} cells detected")
print()

for cell_index in range(num_cells):
    print(f"cell {cell_index} voltage (mV):")
    voltage = int(input("> "))
    res = write_cell_voltage(modbus, cell_index, voltage)
    print("res:", res)

data = get_state(modbus)
print(data)
