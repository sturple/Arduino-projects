#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "ST25DVSensor.h"
#include <Adafruit_PN532.h>





#define LED 25
#define RED_LED 15
#define GREEN_LED 14
#define PIN_WIRE_SDA (4)
#define PIN_WIRE_SCL (5)
#define PN532_IRQ (6)
#define PN532_RESET (7)


Adafruit_LIS3DH lis = Adafruit_LIS3DH();
Adafruit_BME280 bme;
Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);
ST25DV st25dv(12, -1, &Wire);

void setup() {

  Serial.begin(115200);
  while (!Serial) delay(10);
  setup_led();
  setup_st25dv();
  //setup_bme();
  //setup_pn532();
}

void loop() {
  do_led();
  //do_st25dv();
  //do_bme();
  //do_pn532();
}




void setup_pn532() {
  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1)
      ;  // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5");
  Serial.println((versiondata >> 24) & 0xFF, HEX);
  Serial.print("Firmware ver. ");
  Serial.print((versiondata >> 16) & 0xFF, DEC);
  Serial.print('.');
  Serial.println((versiondata >> 8) & 0xFF, DEC);
}

void do_pn532() {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  String readString;
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  // takes value of block to read
  Serial.flush();
  while (Serial.available()) {
    delay(3);  //delay to allow buffer to fill
    if (Serial.available() > 0) {
      char c = Serial.read();  //gets one byte from serial buffer
      readString += c;         //makes the string readString
    }
  }
  Serial.flush();

  if (success) {
    // Display some basic information about the card
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");
    Serial.print(uidLength, DEC);
    Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);

    if (uidLength == 4) {
      // We probably have a Mifare Classic card ...
      uint32_t cardid = uid[0];
      cardid <<= 8;
      cardid |= uid[1];
      cardid <<= 8;
      cardid |= uid[2];
      cardid <<= 8;
      cardid |= uid[3];
      Serial.print("Seems to be a Mifare Classic card #");
      Serial.println(cardid);
      Serial.println(readString.toInt());
    }

    pn532_readblock(uid, uidLength, readString.toInt());
    Serial.println("");
  }
}

void pn532_readblock(uint8_t *uid, uint8_t uidLength, int8_t currentBlock) {
  uint8_t data[32];
  uint8_t keyuniversal[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  uint8_t success;
  digitalWrite(GREEN_LED, HIGH);

  if (uidLength == 4) {
    Serial.print("Trying to authenticate block ");
    Serial.println(currentBlock);
    success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, currentBlock, 0, keyuniversal);
    if (success) {
      Serial.println("Block has been authenticated");
      success = nfc.mifareclassic_ReadDataBlock(currentBlock, data);
    } else {
      Serial.println("Ooops ... authentication failed: Try another key?");
      digitalWrite(RED_LED, HIGH);
    }
  } else {
    success = nfc.mifareultralight_ReadPage(currentBlock, data);
  }

  if (success) {
    // Data seems to have been read ... spit it out
    Serial.print("Reading Block ");
    Serial.println(currentBlock);
    nfc.PrintHexChar(data, 16);
    Serial.println("");
    // Wait a bit before reading the card again
    delay(1000);
  } else {
    Serial.print("Ooops ... unable to read the requested block ");
    digitalWrite(RED_LED, HIGH);
    Serial.println(currentBlock);
  }
}


void setup_st25dv() {
  const char uri_write_message[] = "st.com/st25";        // Uri message to write in the tag
  const char uri_write_protocol[] = URI_ID_0x01_STRING;  // Uri protocol to write in the tag
  String uri_write = String(uri_write_protocol) + String(uri_write_message);
  if (st25dv.begin() == 0) {
    Serial.println("System Init done!");
  } else {
    Serial.println("System Init failed!");
    while (1)
      ;
  }
}

void do_st25dv() {
  // if(st25dv.writeURI(uri_write_protocol, uri_write_message, "")) {
  //   Serial.println("Write failed!");
  //   while(1);
  // }

  if (st25dv.writeEMail("shawn@turple.ca", "my subject from tag", "my message abc 123", "info")) {
    Serial.println("Write failed");
    while (1)
      ;
  }
}

void setup_bme() {
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1) yield();
  }
}

void do_bme() {
  Serial.print("temperature:");
  Serial.print((String)bme.readTemperature());
  Serial.print(",");
  Serial.print("pressure:");
  Serial.print((String)bme.readPressure());
  Serial.print(",");
  Serial.print("Humitity:");
  Serial.print((String)bme.readHumidity());
  Serial.print(",");
  Serial.print("Altitude:");
  Serial.print((String)bme.readAltitude(1013));
  Serial.println("");
}

void setup_led() {
  pinMode(LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
}

void do_led() {
  digitalWrite(LED, HIGH);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  delay(100);
  digitalWrite(LED, LOW);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
  delay(1000);
}