# SPDX-FileCopyrightText: 2021 ladyada for Adafruit Industries
# SPDX-License-Identifier: MIT

import busio
from busio import I2C
import board
from adafruit_bus_device.i2c_device import I2CDevice
from micropython import const

_BME280_ADDRESS = const(0x77)
_BME280_CHIPID = const(0x60)
_BME280_REGISTER_CHIPID = const(0xD0)
_BME280_REGISTER_TEMPDATA = const(0xFA)

class I2C_Impl:
    "Protocol implementation for the I2C bus."

    def __init__(self, i2c: I2C, address: int) -> None:
        from adafruit_bus_device import (  # pylint: disable=import-outside-toplevel
            i2c_device,
        )

        self._i2c = i2c_device.I2CDevice(i2c, address)

    def read_register(self, register: int, length: int) -> bytearray:
        "Read from the device register."
        with self._i2c as i2c:
            write_buffer = bytearray(2)
            write_buffer[0] = register >> 8
            write_buffer[1] = register & 0xFF
            read_buffer = bytearray(length)
            i2c.write_then_readinto(write_buffer, read_buffer)
            return read_buffer

    def write_register_byte(self, register: int, value: int) -> None:
        """Write to the device register"""
        with self._i2c as i2c:
            i2c.write(bytes([register & 0xFF, value & 0xFF]))


class ST25DV_I2C:
    def __init__(self, bus_implementation) -> None:
        """Something"""
        self._bus_implementation = bus_implementation
        print( (self._read_register(0x00,0x100)) )
        #chip_id = self._read_register(0xA6, 8)
        #print((chip_id))

    def _read_byte(self, register: int) -> int:
        """Read a byte register value and return it"""
        return self._read_register(register, 1)[0]

    def _read24(self, register: int) -> float:
        """Read an unsigned 24-bit value as a floating point and return it."""
        ret = 0.0
        for b in self._read_register(register, 3):
            ret *= 256.0
            ret += float(b & 0xFF)
        return ret

    def _read_register(self, register: int, length: int) -> bytearray:
        return self._bus_implementation.read_register(register, length)

    def _write_register_byte(self, register: int, value: int) -> None:
        self._bus_implementation.write_register_byte(register, value)



DEVICE_ADDRESS = 0x50  # 0x2D / 0x53 / 0x57  st25dv device address of DS3231 board



st25 = ST25DV_I2C(I2C_Impl(busio.I2C(board.SCL, board.SDA), DEVICE_ADDRESS))

'''
A_DEVICE_REGISTER = 0xD0  # device id register on the DS3231 board
# The follow is for I2C communications
comm_port = busio.I2C(board.SCL, board.SDA)
device = I2CDevice(comm_port, DEVICE_ADDRESS)
st25 = I2C_Impl(comm_port, 0x24);
print(st25.read_register(0,16));


with device as bus_device:
    bus_device.write(bytes([A_DEVICE_REGISTER]))
    result = bytearray(1)
    bus_device.readinto(result)

print("".join("{:02x}".format(x) for x in result))
'''
