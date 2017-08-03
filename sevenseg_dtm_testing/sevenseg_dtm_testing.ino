/*
Kelly Brennan and Celine Ta
July 28th, 2017

Initial test code for seven segment display, membrane switch (simulating an up arrow button), 
and Arduino Micro. When membrane button is pressed, the display increases the "set temperature" 
by 1 degree, unless it is already at the maxTemp.
*/


#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

Adafruit_7segment matrix = Adafruit_7segment();

// temp parameters
int setTemp = 34; 
int upTemp = 0;
int minTemp = 28; 
int maxTemp = 38; 

void setup() {
  // put your setup code here, to run once:
#ifndef __AVR_ATtiny85__
  Serial.begin(9600);
  Serial.println("7 Segment Backpack Test");
#endif
  matrix.begin(0x70); // begins I2C communication with seven segment display
  matrix.setBrightness(3); // sets brightness, on scale of 0 (dim) to 15 (bright)
}

void loop() {
  // put your main code here, to run repeatedly:

  upTemp = analogRead(A0); // get signal fom membrane switch (up arrow)
  if (upTemp >= 1000 && setTemp < maxTemp) {  // condition that membrane switch is high (pressed) and that the desired set temp is less than max
//    insert debouncing code here if necessary
    setTemp = setTemp+1; // increment set temperature by 1
    delay(400); // debouncing
  }
//  Serial.print("Analog Read: "); 
//  Serial.print(upTemp); 
//  Serial.print(" Set Temp: "); 
//  Serial.println(setTemp);   
  
  matrix.print(setTemp);
  matrix.writeDisplay(); // refreshes display with new content
}
