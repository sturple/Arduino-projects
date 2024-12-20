#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
Adafruit_BME280 bme;


#define LED 25
#define RED_LED 15
#define GREEN_LED 14
#define PIN_WIRE_SDA        (4)
#define PIN_WIRE_SCL        (5)

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(RED_LED, HIGH);
  Serial.begin(115200);
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1) yield();
  }
  
}

void loop() {
  led_loop();
  plotVariables();
  
}

void print_bme() {

}


void plotVariables() {
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

void led_loop() {
  digitalWrite(LED, HIGH);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  delay(100);
  digitalWrite(LED, LOW);
  digitalWrite(RED_LED, HIGH);
  digitalWrite(GREEN_LED, LOW);
  delay(1000);
}