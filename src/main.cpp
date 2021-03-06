//Library HX711 by Bogdan Necula: https://github.com/bogde/HX711
// Library: pushbutton by polulu: https://github.com/pololu/pushbutton-arduino

#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Pushbutton.h>
#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <config.h> 


HX711 scale;
float reading; //this can be "int" as well
float lastReading; //this can be "int" as well

// Create InfluxDB client instance
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert); 
// Data point for influxDB
Point sensor("force_gauge");

//OLED Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Button
Pushbutton button(BUTTON_PIN);

void displayWeight(float weight){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  // Display static text
  display.println("Weight:");
  display.display();
  display.setCursor(0, 30);
  display.setTextSize(2);
  display.print(weight);
  display.print(" ");
  display.print("kg");
  display.display();  
}

void setup() {
  Serial.begin(115200);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextColor(WHITE);

      // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PW);

  Serial.println("Connecting to wifi...");
    if(wifiMulti.run() == WL_CONNECTED){
      Serial.print("Connected to ");
      Serial.println(WiFi.SSID());
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
     }
  else
     {
       Serial.println("WiFi connection failed to SSID: ");
       Serial.print(WIFI_SSID);
       Serial.println("Using WIFI Password: ");
       Serial.print(WIFI_PW);
       exit(1);
     }

  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println(CALIBRATION_FACTOR);
  scale.set_scale(CALIBRATION_FACTOR);   
  scale.tare();              

  // Add constant tags - only once
  sensor.addTag("device", DEVICE_NAME + WiFi.macAddress());

  // Check server connection
  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
   }else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  //Set our time so our output data is accurate
  timeSync(TZ, "pool.ntp.org", "time.nis.gov");
}

void loop() {
  // If no Wifi signal, try to reconnect it
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Wifi connection lost");
   }
   //Tare button command 
  if (button.getSingleDebouncedPress()){
    Serial.print("tare...");
    scale.tare();
  }
  //Display weight reading on LED, shows constant numbers until the weight changes
  if (scale.wait_ready_timeout(200)) {
    reading = (scale.get_units()); //add "round" to get whole numbers on display
    Serial.print("Weight: ");
    Serial.println(reading);
    if (reading != lastReading){
      displayWeight(reading); 
    }
    lastReading = reading;
  }
  else {
    Serial.println("HX711 not found.");
  }
// Store measured value into point
   sensor.clearFields();
   // Print the scale reading
   sensor.addField("weight", reading);
   // Print what are we exactly writing
   Serial.print("Writing: ");
   Serial.println(client.pointToLineProtocol(sensor));

   // Write point
   if (!client.writePoint(sensor)) {
     Serial.print("InfluxDB write failed: ");
     Serial.println(client.getLastErrorMessage());
   }
   delay(100);
}