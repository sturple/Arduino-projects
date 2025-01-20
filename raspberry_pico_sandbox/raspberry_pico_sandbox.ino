#include "pins_arduino.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "ST25DVSensor.h"
#include <Adafruit_PN532.h>
#include "RF24.h"


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

 
#define CE_PIN 28
#define CSN_PIN 17
RF24 radio(CE_PIN, CSN_PIN);
 
// Let these addresses be used for the pair
uint8_t address[][6] = { "1Node", "2Node" };
// It is very helpful to think of an address as a path instead of as
// an identifying device destination
 
// to use different addresses on a pair of radios, we need a variable to
// uniquely identify which address this radio will use to transmit
bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit
 
// Used to control whether this node is sending or receiving
bool role = false;  // true = TX role, false = RX role
 
// For this example, we'll be using a payload containing
// a single float number that will be incremented
// on every successful transmission
float payload = 0.0;
#define RND_MAXLENGTH     256

#define SUPPRESSCOLLORS

void setup() {

  Serial.begin(115200);
  while (!Serial) delay(10);
  setup_led();
  //setup_st25dv();
  //setup_bme();
  //setup_pn532();
  setup_nRF24L01();
}

void loop() {
  do_led();
  //do_st25dv();
  //do_bme();
  //do_pn532();
  do_nRF24L01();
}

void setup_nRF24L01() {
  Serial.println("Setting up 2.4 Wireless device nR24L01");
  Serial.println(MISO);
  Serial.println(MOSI);
  Serial.println(SCK);
  Serial.println(SS);

// initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }
 
  // print example's introductory prompt
  Serial.println(F("RF24/examples/GettingStarted"));
 
  // To set the radioNumber via the Serial monitor on startup
  Serial.println(F("Which radio is this? Enter '0' or '1'. Defaults to '0'"));
  while (!Serial.available()) {
    // wait for user input
  }
  char input = Serial.parseInt();
  radioNumber = input == 1;
  Serial.print(F("radioNumber = "));
  Serial.println((int)radioNumber);
 
  // role variable is hardcoded to RX behavior, inform the user of this
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));
 
  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.
 
  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(payload));  // float datatype occupies 4 bytes
 
  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[radioNumber]);  // always uses pipe 0
 
  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1
 
  // additional setup specific to the node's role
  if (role) {
    radio.stopListening();  // put radio in TX mode
  } else {
    radio.startListening();  // put radio in RX mode
  }
 
  // For debugging info
  // printf_begin();             // needed only once for printing details
  // radio.printDetails();       // (smaller) function that prints raw register values
  // radio.printPrettyDetails(); // (larger) function that prints human readable data  
}

void do_nRF24L01() {
  Serial.println("doing 2.4 Wireless device");
    if (role) {
    // This device is a TX node
 
    unsigned long start_timer = micros();                // start the timer
    bool report = radio.write(&payload, sizeof(float));  // transmit & save the report
    unsigned long end_timer = micros();                  // end the timer
 
    if (report) {
      Serial.print(F("Transmission successful! "));  // payload was delivered
      Serial.print(F("Time to transmit = "));
      Serial.print(end_timer - start_timer);  // print the timer result
      Serial.print(F(" us. Sent: "));
      Serial.println(payload);  // print payload sent
      payload += 0.01;          // increment float payload
    } else {
      Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
    }
 
    // to make this example readable in the serial monitor
    delay(1000);  // slow transmissions down by 1 second
 
  } else {
    // This device is a RX node
 
    uint8_t pipe;
    if (radio.available(&pipe)) {              // is there a payload? get the pipe number that received it
      uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
      radio.read(&payload, bytes);             // fetch payload from FIFO
      Serial.print(F("Received "));
      Serial.print(bytes);  // print the size of the payload
      Serial.print(F(" bytes on pipe "));
      Serial.print(pipe);  // print the pipe number
      Serial.print(F(": "));
      Serial.println(payload);  // print the payload's value
    }
  }  // role
 
  if (Serial.available()) {
    // change the role via the serial monitor
 
    char c = toupper(Serial.read());
    if (c == 'T' && !role) {
      // Become the TX node
 
      role = true;
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      radio.stopListening();
 
    } else if (c == 'R' && role) {
      // Become the RX node
 
      role = false;
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
      radio.startListening();
    }
  }
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