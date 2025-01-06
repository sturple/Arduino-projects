
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
        self._print((current_block, data))
        return self._pn532.mifare_classic_write_block(current_block, data)
    

    def _get_data_block_start_from_sector(self, current_sector) -> list:
        # sector 0 - 3 (trailer), 2-1 (data), 0 (manufactures info)
        # sector 1 - 7 (trailer), 6-4 (data)
        trailer_block = current_sector * 4 + 3
        return (trailer_block, trailer_block - 3, trailer_block - 2, trailer_block - 1 )
        

    def read_card_sector(self, uid, current_sector, passkey=b'\xFF\xFF\xFF\xFF\xFF\xFF') -> tuple:
        """reads the sectors 1-15 (4)"""
        block_address_list = self._get_data_block_start_from_sector(current_sector)
        trailer_block, data3_block, data2_block, data1_block = block_address_list
        data = []
        sector_trailer = None
        if self._authenticate_card_block(uid, trailer_block, passkey):
            sector_trailer  = self._read_card_block(trailer_block)
            if 0 != current_sector:
                data.append(self._read_card_block(data3_block))
         
            data.append(self._read_card_block(data2_block))
            data.append(self._read_card_block(data1_block))   
            data.append( sector_trailer[0:6] )

        return (current_sector, sector_trailer, data, block_address_list)
    

    def write_card_sector(self, uid, current_sector, data, passkey=b'\xFF\xFF\xFF\xFF\xFF\xFF'):
        """writes to a card sector"""
        block_address_list = self._get_data_block_start_from_sector(current_sector)
        trailer_block, data3_block, data2_block, data1_block = block_address_list
        if self._authenticate_card_block(uid, trailer_block, passkey):
            if 0 != current_sector:
                self._write_card_block(data3_block, data[2][0])
                self._write_card_block(data2_block, data[2][1])
                self._write_card_block(data1_block, data[2][2])

    
    def _print(self, msg)-> None:
        if self._debug:
            print(msg)

    def diagnostic(self, current_sector) -> None:
        self._print(self.get_version())
        self._print('Read Card')
        uid = self.read_card()
        self._print(uid)
        passkey = b'\xFF\xFF\xFF\xFF\xFF\xFF'
        data = self.read_card_sector(uid, current_sector, passkey)
        # self._print('Original data')
        self._print(data)
        # self._print('write data')
        # data[2][0] = b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
        # self.write_card_sector(uid, current_sector, data, passkey)
   
           


        

    
if __name__ == "__main__":
    pn = PN532(debug=True)
    pn.diagnostic(1)