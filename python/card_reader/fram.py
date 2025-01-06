# SPDX-FileCopyrightText: 2021 ladyada for Adafruit Industries
# SPDX-License-Identifier: MIT

## Simple Example For CircuitPython/Python I2C FRAM Library

import board
import busio
import adafruit_fram

class Fram():

    _fram = None
    _debug = False
    _max_address = 0x8000

    def __init__(self, debug=False) -> None:
        i2c = busio.I2C(board.SCL, board.SDA)
        self._fram = adafruit_fram.FRAM_I2C(i2c)
        self._debug = debug

    def read(self, start_addr=0, length=1)->bytearray:
        end_addr = min(start_addr+max(length,1), self._max_address)
        return self._fram[start_addr:end_addr]
    
    def write(self, start_addr=0, data=b'0x00')->bool:
        end_addr = min(start_addr+max(1, len(data)), self._max_address)
        self._fram[start_addr:end_addr] = data
        return self._fram[start_addr:end_addr]
        
    def _print(self, msg)-> None:
        if self._debug:
            print(msg)    

    def diagnostic(self) -> None:
   
        self._print(self.read(0x0,0x7000 ))
        #self._print(self.write(0x100,b'shawn turple\x00'))


if __name__ == "__main__":
    fram = Fram(debug=True)
    fram.diagnostic()