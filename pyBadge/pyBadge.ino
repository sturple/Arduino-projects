
#include <Wire.h>
#define NEOPIXEL_ENABLE true
#define AUDIO_ENABLE true
#ifdef NEOPIXEL_ENABLE
  #include <Adafruit_NeoPixel.h>
  #define PIN 8
  #define NUM_PIXELS 5 
  Adafruit_NeoPixel pixels(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
#endif
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#ifdef AUDIO_ENABLE
  #define ARCADA_AUDIO_OUT A0
  #define ARCADA_SPEAKER_ENABLE 51
#endif 


#define BUTTON_CLOCK 48
#define BUTTON_DATA 49
#define BUTTON_LATCH 50
#define LIGHT_SENSOR A7
#define BUTTON_SHIFTMASK_B 0x80
#define BUTTON_SHIFTMASK_A 0x40
#define BUTTON_SHIFTMASK_START 0x20
#define BUTTON_SHIFTMASK_SELECT 0x10
#define BUTTON_SHIFTMASK_LEFT 0x01
#define BUTTON_SHIFTMASK_UP 0x02
#define BUTTON_SHIFTMASK_DOWN 0x04
#define BUTTON_SHIFTMASK_RIGHT 0x08

#define BUTTONMASK_A 0x01
#define BUTTONMASK_B 0x02
#define BUTTONMASK_SELECT 0x04
#define BUTTONMASK_START 0x08
#define BUTTONMASK_UP 0x10
#define BUTTONMASK_DOWN 0x20
#define BUTTONMASK_LEFT 0x40
#define BUTTONMASK_RIGHT 0x80

bool debounce_flag = false;

void setup()
{
  
  Wire.begin(0x72);              // join i2c bus with address #0x72
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(requestEvent);
  Serial.begin(115200);           // start serial for output
  pinMode(13, OUTPUT);

  pinMode(BUTTON_CLOCK, OUTPUT);
  digitalWrite(BUTTON_CLOCK, HIGH);
  pinMode(BUTTON_LATCH, OUTPUT);
  digitalWrite(BUTTON_LATCH, HIGH);
  pinMode(BUTTON_DATA, INPUT);
  pinMode(LIGHT_SENSOR, INPUT_PULLUP);
  pinMode(ARCADA_AUDIO_OUT, OUTPUT);
  pinMode(ARCADA_SPEAKER_ENABLE, OUTPUT);

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif    


  #ifdef NEOPIXEL_ENABLE
  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.setBrightness(1);
  #endif
  digitalWrite(ARCADA_SPEAKER_ENABLE, HIGH);
  tone(ARCADA_AUDIO_OUT, 3000, 100);

}

void loop() {
  setNeoPixel(0, 111, 255, 111);
  delay(500);
  setNeoPixel(0, 255, 0, 255);
  delay(500);
}

// function that executes whenever data is received from master
// this function is registered as an event, see setup()
void receiveEvent(int howMany) {
  digitalWrite(13, 1);
  while (1 < Wire.available()) { // loop through all but the last
    char c = Wire.read(); // receive byte as a character
    //Serial.println((c));    // print the character
  }
  int x = Wire.read();    // receive byte as an integer
  digitalWrite(13, 0);
  
}

void requestEvent()
{
  uint16_t data = analogRead(LIGHT_SENSOR);
  uint8_t lsb = data & 0xff;
  uint8_t msb = data >>= 8;
  Wire.write(msb);
  Wire.write(lsb);
  uint32_t bt = readButtons();
  if (bt > 0 ){
    digitalWrite(ARCADA_SPEAKER_ENABLE, HIGH);
    tone(ARCADA_AUDIO_OUT, 4000 , 10);
  }
  Wire.write(bt);
 
  
}

#ifdef NEOPIXEL_ENABLE
void setNeoPixel( byte item, uint8_t colorRed, uint8_t colorGreen, uint8_t colorBlue ) {
  pixels.setPixelColor(item, pixels.Color(colorRed, colorGreen, colorBlue));
  pixels.show();   // Send the

}
#endif

uint32_t readButtons() {
  uint32_t buttons = 0;

  // Use a latch to read 8 bits
  if (BUTTON_CLOCK >= 0) {
    uint8_t shift_buttons = 0;
    digitalWrite(BUTTON_LATCH, LOW);
    delayMicroseconds(1);
    digitalWrite(BUTTON_LATCH, HIGH);
    delayMicroseconds(1);

    for (int i = 0; i < 8; i++) {
      shift_buttons <<= 1;
      shift_buttons |= digitalRead(BUTTON_DATA);
      digitalWrite(BUTTON_CLOCK, HIGH);
      delayMicroseconds(1);
      digitalWrite(BUTTON_CLOCK, LOW);
      delayMicroseconds(1);
    }
    if (shift_buttons & BUTTON_SHIFTMASK_B)
      buttons |= BUTTONMASK_B;

    if (shift_buttons & BUTTON_SHIFTMASK_A)
      buttons |= BUTTONMASK_A;
  
    if (shift_buttons & BUTTON_SHIFTMASK_SELECT)
      buttons |= BUTTONMASK_SELECT;

    if (shift_buttons & BUTTON_SHIFTMASK_START)
      buttons |= BUTTONMASK_START;

    if (shift_buttons & BUTTON_SHIFTMASK_UP)
      buttons |= BUTTONMASK_UP;

    if (shift_buttons & BUTTON_SHIFTMASK_DOWN)
      buttons |= BUTTONMASK_DOWN;
    
    if (shift_buttons & BUTTON_SHIFTMASK_LEFT)
      buttons |= BUTTONMASK_LEFT;

    if (shift_buttons & BUTTON_SHIFTMASK_RIGHT)
      buttons |= BUTTONMASK_RIGHT;

  }
  return buttons;
}

