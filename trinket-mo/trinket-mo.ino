//#define RTC_ENABLE true
//#define LIS3DH_ENABLE true
#define DOTSTAR_ENABLE true
#include <Wire.h>

#ifdef RTC_ENABLE
  #include <DS3231.h>
  DS3231 myRTC;
  #define RTC_ADDRESS 0x68
  bool century = false;
  bool h12Flag;
  bool pmFlag;
#endif

#ifdef LIS3DH_ENABLE
  #include <Adafruit_LIS3DH.h>
  #include <Adafruit_Sensor.h>
  Adafruit_LIS3DH lis = Adafruit_LIS3DH();
#endif

#ifdef DOTSTAR_ENABLE
  #include <Adafruit_DotStar.h>
  Adafruit_DotStar strip(DOTSTAR_NUM, PIN_DOTSTAR_DATA, PIN_DOTSTAR_CLK, DOTSTAR_BRG);
#endif

String serialMsg;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);
  Wire.begin();  
  pinMode(13, OUTPUT);

  // Dotstar setup
  #ifdef DOTSTAR_ENABLE
  strip.begin(); // Initialize pins for output
  strip.setBrightness(10);
  strip.show();  // Turn all LEDs off ASAP
  #endif

  // LIS3DH Accelomoter setup
  #ifdef LIS3DH_ENABLE
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1) yield();
  }
  Serial.println("LIS3DH found!");
  #endif
  
}


void loop() {
  // put your main code here, to run repeatedly:
  writePyBadgeWire();
  readPyBadgeWire();
  readSerialIn();

  #ifdef RTC_ENABLE
  reaRawdRTC();
  delay(100);
  #endif

  #ifdef LIS3DH_ENABLE
  readLIS();
  #endif

  #ifdef DOTSTAR_ENABLE
  //rainbow(10);
  #endif
  
}

void readSerialIn() {
  if (Serial.available() > 0) {
    char c = Serial.read();
    if ( c == 10 ) {
      Serial.println(decodeMsg(serialMsg));
      serialMsg = "";
    } else {
      serialMsg.concat(c);
    }
  } 
}

String decodeMsg( String msg ) {
  if ( msg == "hello" ) {
    return "hello back";
  } else if ( msg == "get all" ){
    return "get all";
  } else if ( msg == "clock" ){
    return "";
  }
  return  "sorry i didn't understand your requests "+ msg ;
}


void readPyBadgeWire() {
  byte bufferCount = 3;
  uint16_t buffer[bufferCount];
  byte x = 0;
  Wire.requestFrom(0x72,bufferCount);    // request 6 bytes from slave device #2
  while(Wire.available()) { 
    buffer[x++] = Wire.read(); // receive a byte as character
  }
  // Button has been pressed
  if (buffer[2] > 0) {
    for ( byte y = 0; y < bufferCount; y++){
      Serial.print(buffer[y], HEX); Serial.print(" ");
    }
    Serial.println("");
  }
}

void writePyBadgeWire() {
  Wire.beginTransmission(0x72); // transmit to device #8
  Wire.write("Someting");        // sends five bytes
  Wire.endTransmission();    // stop transmitting

}





#ifdef DOTSTAR_ENABLE
void rainbow(int wait) {
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}
#endif

#ifdef LIS3DH_ENABLE
void readLIS() {
  lis.read();      // get X Y and Z data at once
  // Then print out the raw data
  Serial.print("X:  "); Serial.print(lis.x);
  Serial.print("  \tY:  "); Serial.print(lis.y);
  Serial.print("  \tZ:  "); Serial.print(lis.z);

  /* Or....get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("\t\tX: "); Serial.print(event.acceleration.x);
  Serial.print(" \tY: "); Serial.print(event.acceleration.y);
  Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
  Serial.println(" m/s^2 ");

  Serial.println();
}
#endif


/** RTC functions Start **/
#ifdef RTC_ENABLE
void rtc() {
  Serial.print(myRTC.getYear(), DEC);
  Serial.print("-");
  Serial.print(myRTC.getMonth(century), DEC);
  Serial.print("-");
  Serial.print(myRTC.getDate(), DEC);
  Serial.print(" ");
  Serial.print(myRTC.getHour(h12Flag, pmFlag), DEC); //24-hr
  Serial.print(":");
  if ( myRTC.getMinute() < 10 ){
    Serial.print("0");
  }
  Serial.print(myRTC.getMinute(), DEC);
  Serial.print(":");
  if ( myRTC.getSecond() < 10 ){
    Serial.print("0");
  }
  Serial.println(myRTC.getSecond(), DEC);
}


void reaRawdRTC() {
  //1101000
  //  Wire.beginTransmission(CLOCK_ADDRESS);
	//  Wire.write(0x02);
  // Wire.write(0);//seconds
  // Wire.write(0x25); //minutes
  // Wire.write(0x55); //hour
  // Wire.write(7); //dayofweek
  // Wire.write(0x19); //date
  // Wire.write(0x01); //month
  // Wire.write(0x25); //year

  // Wire.write(0x00); //a1 seconds
  // Wire.write(0x45); //a1 minute
  // Wire.write(0x80); //a1 hour
  // Wire.write(0x80); //a1 day
  //Wire.endTransmission();

  Wire.beginTransmission(RTC_ADDRESS);
	Wire.write(0x00);
	Wire.endTransmission();

  Wire.requestFrom(RTC_ADDRESS,0x15);    // request 6 bytes from slave device #2
  while(Wire.available()) { 
    char c = Wire.read();
    Serial.print(c, HEX); Serial.print(" ");
  }
  Serial.println("");
  
  
}

#endif
/** RTC functions End **/
