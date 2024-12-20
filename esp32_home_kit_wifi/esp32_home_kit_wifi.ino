#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
Adafruit_LIS3DH lis = Adafruit_LIS3DH();
Adafruit_BME280 bme;

const int identity_led = 25;
const int led_gpio = 13;
const int red_gpio = 26;
const int MAX_ANALOG_VAL = 4095;
const float MAX_BATTERY_VOLTAGE = 4.2;  // Max LiPoly voltage of a 3.7 battery is 4.2



extern "C" {
#include "homeintegration.h"
}

homekit_service_t* hapservice = { 0 };
homekit_service_t* temperature = NULL;
homekit_service_t* humidity = NULL;
homekit_server_config_t* config = { 0 };
//homekit_service_t* st_hap_add_temperature_service(const char* szname);
//homekit_service_t* battery = NULL;
String pair_file_name = "/pair-sensors.dat";

// Enter your WiFi SSID and password
char ssid[] = "big-island";       // your network SSID (name)
//char pass[] = "T93ZKPSEH4";  // your network password (use for WPA, or use as key for WEP)
char pass[] = "brandonturple21";
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int status = WL_IDLE_STATUS;
float batteryLevel = 0;


#define uS_TO_S_FACTOR 1000000ULL /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 5           /* Time ESP32 will go to sleep (in seconds) */

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClient client;
WiFiServer server(80);

void setup() {
  // Setup IO devices.
  io_init();
  // Setup Apple home kit.
  hap_init();
}


void loop() {
  sensors();
  webServer();
  delay(1000);
  plotVariables();

  //esp_sleep_enable_wifi_wakeup();
  //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  //Serial.flush();
  //delay(5000);
  //esp_light_sleep_start();
}

void sensors() {
  lis.read();
  setBatteryLevel();
  if (temperature) {
    homekit_characteristic_t* ch_temp = homekit_service_characteristic_by_type(temperature, HOMEKIT_CHARACTERISTIC_CURRENT_TEMPERATURE);
    homekit_characteristic_t* ch_temp_status = homekit_service_characteristic_by_type(temperature, HOMEKIT_CHARACTERISTIC_STATUS_ACTIVE);

    if (ch_temp && !isnan(bme.readTemperature()) && ch_temp->value.float_value != bme.readTemperature()) {
      ch_temp->value.float_value = bme.readTemperature();
      homekit_characteristic_notify(ch_temp, ch_temp->value);
    }

    if (ch_temp_status) {
      if (ch_temp_status->value.bool_value != digitalRead(led_gpio)) {  //will notify only if different
        ch_temp_status->value.bool_value = digitalRead(led_gpio);
        homekit_characteristic_notify(ch_temp_status, ch_temp_status->value);
      }
    }
  }
  if (humidity) {
    homekit_characteristic_t* ch_hum = homekit_service_characteristic_by_type(humidity, HOMEKIT_CHARACTERISTIC_CURRENT_RELATIVE_HUMIDITY);

    if (ch_hum && !isnan(bme.readHumidity()) && ch_hum->value.float_value != bme.readHumidity()) {
      ch_hum->value.float_value = bme.readHumidity();
      homekit_characteristic_notify(ch_hum, ch_hum->value);
    }
  }
}

// Connect all the IO devices.
void io_init() {
  pinMode(led_gpio, OUTPUT);
  pinMode(red_gpio, OUTPUT);
  Serial.begin(115200);
  // Mount SPIFFS file system
  if (!SPIFFS.begin(true)) {
    Serial.print("SPIFFS mount failed");
  }
  // put your setup code here, to run once:
  if (!lis.begin(0x18)) {  // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt find LIS3DH on 0x18");
    while (1) yield();
  }
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1)
      ;
  }
  Serial.print("SSID: ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println('connected');
  server.begin();
  printWifiStatus();
}

// Initialization for Apple Home Kit.
void hap_init() {
  hap_set_identity_gpio(identity_led);    //identity_led=2 will blink on identity
  init_hap_storage();
  set_callback_storage_change(storage_changed);
  hap_setbase_accessorytype(homekit_accessory_category_sensor);
  // init base properties
  hap_initbase_accessory_service("shawn", "turpHK", "222", "EspHapLed", "1.0");
  //hap_initbase_accessory_service(HOSTNAME,"turpHK","0","EspHapLed","1.0");

  hapservice = hap_add_lightbulb_service("Led", led_callback, (void*)&led_gpio);
  
  temperature = hap_add_temperature_service("Temperature");
  // homekit_add_characteristic_to_service(temperature, NEW_HOMEKIT_CHARACTERISTIC(
  //                                                      STATUS_ACTIVE2,
  //                                                      true,
  //                                                      .type = "00000075-0000-1000-8000-0026BB765291"));


  humidity = hap_add_humidity_service("Humidity");
  //battery = hap_add_battery_service("Battery", battery_callback, NULL);

  // Changing password.
  config = hap_get_server_config();
  hap_set_device_password("111-11-111");
  hap_init_homekit_server();
}

// Initializing the storage for Apple Home kit.
void init_hap_storage() {
  Serial.print("init_hap_storage");
  File fsDAT = SPIFFS.open(pair_file_name, "r");
  if (!fsDAT) {
    Serial.println("Failed to read file pair.dat");
    return;
  }
  int size = hap_get_storage_size_ex();
  char* buf = new char[size];
  memset(buf, 0xff, size);
  int readed = fsDAT.readBytes(buf, size);
  Serial.print("Readed bytes ->");
  Serial.println(readed);
  hap_init_storage_ex(buf, size);
  fsDAT.close();
  delete[] buf;
}

// Wifi Status information.
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void webServer() {
  client = server.available();
  if (client) {
    // an http request ends with a blank line
    bool currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        homekit_characteristic_t* ch = homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);

        long rssi = WiFi.RSSI();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          //client.println("Refresh: 10");        // refresh the page automatically every 5 sec
          client.println();
          client.print("{");
          printJson(client, "x", (String)lis.x, ",");
          printJson(client, "y", (String)lis.y, ",");
          printJson(client, "z", (String)lis.z, ",");
          printJson(client, "temperature", (String)bme.readTemperature(), ",");
          printJson(client, "pressure", (String)bme.readPressure(), ",");
          printJson(client, "humidity", (String)bme.readHumidity(), ",");
          printJson(client, "battery", (String)batteryLevel, ",");
          printJson(client, "light", ch->value.bool_value ? "on" : "off", ",");


          // for (byte idx = 0; idx < sizeof(temperature->characteristics); idx++) {
          //   if (temperature->characteristics[idx] != NULL) {
          //     printJson(client, "type-" + (String)idx, (String)temperature->characteristics[idx]->type, ",");
          //     Serial.printf("type: %s", (String)temperature->characteristics[idx]->type);
          //   }
          // }
          
          printJson(client, "rssi", (String)WiFi.RSSI(), "");
          client.print("}");
          break;
        }

        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}

void printJson(WiFiClient client, String name, String value, String comma) {
  client.print("\"" + name + "\":");
  client.print("\"" + value + "\"" + comma);
}

// Callback if the configuration for Apple home kit is changed,
void storage_changed(char* szstorage, int size) {
  SPIFFS.remove(pair_file_name);
  File fsDAT = SPIFFS.open(pair_file_name, "w+");
  if (!fsDAT) {
    Serial.println("Failed to open pair.dat");
    return;
  }
  fsDAT.write((uint8_t*)szstorage, size);
  fsDAT.close();
}

//can be used for any logic, it will automatically inform Apple Home app about state changes
void set_led(bool val) {
  digitalWrite(led_gpio, val ? HIGH : LOW);
  //we need notify apple about changes
  if (hapservice) {
    //getting on/off characteristic
    homekit_characteristic_t* ch = homekit_service_characteristic_by_type(hapservice, HOMEKIT_CHARACTERISTIC_ON);
    if (ch) {
      if (ch->value.bool_value != val) {  //will notify only if different
        ch->value.bool_value = val;
        homekit_characteristic_notify(ch, ch->value);
      }
    }
  }
}
void battery_callback(homekit_characteristic_t* ch, homekit_value_t value, void* context) {
}

void setBatteryLevel() {
  int rawValue = analogRead(A13);
  float voltageLevel = (rawValue / 4095.0) * 2 * 1.1 * 3.3;  // calculate voltage level
  batteryLevel = voltageLevel / MAX_BATTERY_VOLTAGE * 100;
  // if (battery) {
  //   homekit_characteristic_t* ch_batteryLevel = homekit_service_characteristic_by_type(battery, HOMEKIT_CHARACTERISTIC_BATTERY_LEVEL);
  //   // if (ch_batteryLevel && !isnan(batteryLevel) && ch_batteryLevel->value.float_value != batteryLevel) {
  //   ch_batteryLevel->value.int_value = batteryLevel;
  //   homekit_characteristic_notify(ch_batteryLevel, ch_batteryLevel->value);
  //   // }
  // }
}

void led_callback(homekit_characteristic_t* ch, homekit_value_t value, void* context) {
  Serial.println("led_callback");
  set_led(ch->value.bool_value);
}
void temperatureCallback(homekit_characteristic_t* ch, homekit_value_t value, void* context) {
  Serial.println("temperature callback");
}

void plotVariables() {
  Serial.print("x:");
  Serial.print((String)lis.x);
  Serial.print(",");
  Serial.print("y:");
  Serial.print((String)lis.y);
  Serial.print(",");
  Serial.print("z:");
  Serial.print((String)lis.z);
  Serial.print(",");
  Serial.print("battery:");
  Serial.print((String)batteryLevel);
  Serial.print(",");
  Serial.print("temperature:");
  Serial.print((String)bme.readTemperature());
  Serial.println("");
}



// homekit_service_t* st_hap_add_temperature_service(char* szname) {
//   homekit_service_t* service = NEW_HOMEKIT_SERVICE(
//     TEMPERATURE_SENSOR,
//     .characteristics = (homekit_characteristic_t*[]){
//       NEW_HOMEKIT_CHARACTERISTIC(NAME, szname),
//       NEW_HOMEKIT_CHARACTERISTIC(CURRENT_TEMPERATURE, 0),
//       NEW_HOMEKIT_CHARACTERISTIC(STATUS_ACTIVE, true),
//       NEW_HOMEKIT_CHARACTERISTIC(STATUS_FAULT, true),
//       NEW_HOMEKIT_CHARACTERISTIC(STATUS_LOW_BATTERY, 0),
//       NEW_HOMEKIT_CHARACTERISTIC(STATUS_TAMPERED, 0)
//         NULL });
//   return hap_add_service(service);
// }