

//7-Seg. Libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

//Temperature probe libraries - Using DS18B20 thermometer
#include <DallasTemperature.h> // Temperature probe processing library
#include <OneWire.h> // Temperature probe reading library
#include <math.h> // Arduino library for # processing

//Button Variables
int PWR_BUTTON_PIN = A3; 
int ALARM_BUTTON_PIN = A2; 
int UP_BUTTON_PIN = A1; 
int DOWN_BUTTON_PIN = A0;

int power_last_state = 0; // last read on power button (analog 0 - 1023)
int alarm_last_state = 0; //last read on alarm button (analog 0 - 1023)

//LED Variables
int HOT_LED = 11; // LED pin for "too hot" alarm
int COLD_LED = 12; // LED pin for "too cold" alarm
//int BUZZER = 13; // Buzzer pin

//7-Seg. Display Variables
Adafruit_7segment matrix = Adafruit_7segment();
boolean sevseg_on = false;
//int SEVSEG_PWR_PIN = A5; 
// temp parameters
int setTemp = 34; 
int upTemp = 0;
int downTemp = 0;
int minTemp = 28; 
int maxTemp = 38;

//Temperature reading setup
#define ONE_WIRE_BUS 7
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire); 

float currentTemp; //temperature measurement

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
  pinMode(HOT_LED, OUTPUT); //sets LED pins to output
  pinMode(COLD_LED, OUTPUT); 
  // pinMode(BUZZER, OUTPUT); 
//  pinMode(SEVSEG_PWR_PIN, OUTPUT); 

  //7-Seg. Display
  #ifndef __AVR_ATtiny85_
  Serial.println("7 Segment Backpack Test");
  #endif
  matrix.begin(0x70); // begins I2C communication with seven segment display
  matrix.setBrightness(3); // sets brightness, on scale of 0 (dim) to 15 (bright)
  turnOffDisp();

  //Temperature probe
  sensors.begin(); 
}

void loop() {
// Power button code
  int power_current = analogRead(PWR_BUTTON_PIN); // read power button state
  if (power_current >= 1000 && power_last_state <= 50) {
    if (sevseg_on){
      turnOffDisp(); 
      sevseg_on = false;
    }
    else{
      currentTempUpdate(); 
      sevseg_on = true;
    }
  }
  if (sevseg_on) {
     currentTempUpdate();  
  }
  power_last_state = power_current; // refresh button state in memory

// Alarm button code
  int alarm_current = analogRead(ALARM_BUTTON_PIN);  // read alarm button state
  if (alarm_current >= 1000 && alarm_last_state <= 50) { //if alarm button is pressed anew, min threshold 50 for debouncing (despite pull down) 
    digitalWrite(HOT_LED, !digitalRead(HOT_LED));       //toggled the LED state
  }
  alarm_last_state = alarm_current; // refresh button state in memory
  
// 7-Seg. Display Code
  upTemp = analogRead(UP_BUTTON_PIN); // get signal fom membrane switch (up arrow)
  downTemp = analogRead(DOWN_BUTTON_PIN); //get signal from membrane switch (down arrow)
  if (upTemp >= 1000 && setTemp < maxTemp) {  // condition that membrane switch is high (pressed) and that the desired set temp is less than max
//    insert debouncing code here if necessary
    setTemp = setTemp+1; // increment set temperature by 1
    setTempUpdate();
    delay(400); // debouncing
  }
  if (downTemp >= 1000 && setTemp > minTemp){ //condition that membrane switch is high (pressed) and that desired set temp is greater than min
    setTemp = setTemp-1; // increment set temperature by -1
    setTempUpdate(); 
    delay (400); // debouncing
  }
//  Serial.print("Analog Read: "); 
//  Serial.print(upTemp); 
//  Serial.print(" Set Temp: "); 
//  Serial.println(setTemp);   
}

void setTempUpdate() {
//  matrix.print(setTemp);
  int setTemp_tens = setTemp/10; 
  int setTemp_ones = setTemp%10;
  matrix.writeDigitNum(0, setTemp_tens);
  matrix.writeDigitNum(1, setTemp_ones, false);
  matrix.writeDigitRaw(3,0);
  matrix.writeDisplay(); // refreshes display with new content
  delay(1000);
}

void currentTempUpdate() {
  currentTemp = get_temperature();
  int temp = (int)currentTemp;
  int currentTemp_tens = temp/10; 
  int currentTemp_ones = temp%10; 
  int currentTemp_dec = (currentTemp-temp)*10;
  matrix.writeDigitNum(0, currentTemp_tens); 
  matrix.writeDigitNum(1, currentTemp_ones, true); 
  matrix.writeDigitNum(3, currentTemp_dec); 
  matrix.writeDisplay(); 
}

float get_temperature() { //Receive temperature measurement
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}
void turnOffDisp(){
    for (int i=0; i<=4; i++){
      matrix.writeDigitRaw(i,0); 
    }
    matrix.writeDisplay();
}

