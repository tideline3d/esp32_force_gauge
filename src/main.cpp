//Library HX711 by Bogdan Necula: https://github.com/bogde/HX711
// Library: pushbutton by polulu: https://github.com/pololu/pushbutton-arduino

#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Pushbutton.h>

#include <config.h> 


HX711 scale;
float reading;
float lastReading;


//OLED Display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Pushbutton button(BUTTON_PIN);
Pushbutton button2(BUTTON_PIN2);
Pushbutton button3(BUTTON_PIN3);

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
  display.print("g");
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
  
  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  Serial.println(CALIBRATION_FACTOR);
  scale.set_scale(CALIBRATION_FACTOR);   // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0
}

void loop() {
  if (button.getSingleDebouncedPress()){
    Serial.print("tare...");
    scale.tare();
  }

  if (button2.getSingleDebouncedPress()){
    Serial.print("Power Down");
    delay (2000);
    scale.power_down();
  }

  if (button3.getSingleDebouncedPress())
    reading = (scale.get_units());
    Serial.print("Max weight: ");
    Serial.println(reading);
    if (reading > lastReading){
      displayWeight(reading); 
    }
    lastReading = reading;

  if (scale.wait_ready_timeout(200)) {
    reading = (scale.get_units());
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
}