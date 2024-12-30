
import board
import busio

from digitalio import DigitalInOut
from adafruit_pn532.i2c import PN532_I2C

class PN532():
    """This is the class that interfaces to the PN532 card reader"""
    _pn532 = None
    _debug = False

    def __init__(self, debug=False) -> None:
        """PN532 class constructor"""
        self._debug = debug
        i2c = busio.I2C(board.SCL, board.SDA)
        reset_pin = DigitalInOut(board.G3)
        req_pin = DigitalInOut(board.G2)
        self._pn532 = PN532_I2C(i2c, debug=False, reset=reset_pin, req=req_pin)
        # Configure PN532 to communicate with MiFare cards
        self._pn532.SAM_configuration()

    def read_card(self) -> any:
        #return b'\x00\x00\x00\x01'
        while True:
            uid = self._pn532.read_passive_target(timeout=0.5)
            if uid is None:
                continue
            return uid


    def get_version(self) -> any:
        return self._pn532.firmware_version
    
    def _authenticate_card_block(self, uid, current_block, passkey, key=0x60) -> bool:
        return self._pn532.mifare_classic_authenticate_block(uid, current_block, 0x60, passkey)
    
    def _read_card_block(self, current_block ) -> bytearray:
        return self._pn532.mifare_classic_read_block(current_block)
    
    def _write_card_block(self, current_block, data) -> bool:
        return self._pn532.mifare_classic_write_block(current_block, data)
    

    def _get_data_block_start_from_sector(self, current_sector) -> int:
        return current_sector * 4 - 1

    def read_card_sector(self, uid, current_sector, passkey) -> tuple:
        """reads the sectors 1-15 (4)"""
        data_block_start = self._get_data_block_start_from_sector(current_sector)
        data_list = []
        sectorTrailer = None
        if self._authenticate_card_block(uid, data_block_start, passkey):
            sectorTrailer  = self._read_card_block(data_block_start)
            data_list = [
                self._read_card_block(data_block_start -1),
                self._read_card_block(data_block_start -2),
            ]
            if (1 != current_sector):
                data_list.append(self._read_card_block(data_block_start -3))
        else:
            self._print('read not authenticated')
        return (current_sector, sectorTrailer,  data_list)
    

    def write_card_sector(self, uid, current_sector, data, passkey):
        """writes to a card sector"""
        data_block_start = self._get_data_block_start_from_sector(current_sector)
        if self._authenticate_card_block(uid, data_block_start, passkey):
            if 1 != current_sector:
                self._write_card_block(data_block_start -2, data)

        else:
            self._print('write not authenticated')



    
    
    def _print(self, msg)-> None:
        if self._debug:
            print(msg)

    def diagnostic(self, current_sector) -> None:
        self._print(self.get_version())
        self._print('Read Card')
        uid = self.read_card()
        self._print(uid)
        passkey = b'\xFF\xFF\xFF\xFF\xFF\xFF'
        for x in list(range(1,15)):
            self._print('writing to sector')
            # if 2 == x:
            #     passkey = b'worlds'
            # else:
            #     passkey = b'\xFF\xFF\xFF\xFF\xFF\xFF'
            self.write_card_sector(uid,x, b'worldsailing1245', passkey)
            sector, trailer, data = self.read_card_sector(uid, x, passkey)
            self._print("********Sector {0}".format(sector))
            self._print(trailer)
            self._print(data)
           


        

    
if __name__ == "__main__":
    pn = PN532(debug=True)
    pn.diagnostic(3)