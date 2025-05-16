# pack-monitor

Communicates with 1 or more [cell-monitor](https://github.com/scttnlsn/cell-monitor)s to monitor and
protect a battery pack.

## System Setup (Linux)

```
sudo apt update
sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential g++ libstdc++-arm-none-eabi-newlib
```

## SDK Setup

```
git clone https://github.com/raspberrypi/pico-sdk.git --branch master
cd pico-sdk
git submodule update --init
cd ..
export PICO_SDK_PATH=$(pwd)/pico-sdk
```

## Build

```
mkdir build
cd build
cmake ..
make -j4
```

## References

Pico:

* https://datasheets.raspberrypi.org/pico/Pico-R3-A4-Pinout.pdf
* https://datasheets.raspberrypi.org/rp2040/rp2040-datasheet.pdf
* https://datasheets.raspberrypi.org/pico/raspberry-pi-pico-c-sdk.pdf
* https://raspberrypi.github.io/pico-sdk-doxygen/

Modbus:

* https://www.fernhillsoftware.com/help/drivers/modbus/modbus-protocol.html
* https://minimalmodbus.readthedocs.io/en/stable/modbusdetails.html

Onewire:

* https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html
