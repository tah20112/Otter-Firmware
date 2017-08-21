//#include "pitches.h" // buzzer reference

//7-Seg. Libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"

// Thermistor Parameters
#define THERMISTORNOMINAL 10000 // resistance at 25 degrees C      
#define TEMPERATURENOMINAL 25   // temp. for nominal resistance (almost always 25 C)
#define NUMSAMPLES 5            // how many samples to take and average, more takes longer, but is more "smooth"
#define BCOEFFICIENT 3950       // The beta coefficient of the thermistor (usually 3000-4000)
#define SERIESRESISTOR 10000    // the value of the 'other' resistor

//Button Variables
const int PWR_BUTTON_PIN = A3; 
const int ALARM_BUTTON_PIN = A2; 
const int UP_BUTTON_PIN = A1; 
const int DOWN_BUTTON_PIN = A0;

int power_last_state = 0; // last read on power button (analog 0 - 1023)
int alarm_last_state = 0; //last read on alarm button (analog 0 - 1023)

//LED Variables
const int HOT_LED = 11; // LED pin for "too hot" alarm
const int COLD_LED = 12; // LED pin for "too cold" alarm
//const int BUZZER = 13; // Buzzer pin
const int leftPin = 10; 
const int rightPin = 13;

//7-Seg. Display Variables
Adafruit_7segment matrix = Adafruit_7segment();
boolean sevseg_on = false;

// temp parameters
int setTemp = 34; 
int upTemp = 0;
int downTemp = 0;
int minTemp = 28; 
int maxTemp = 38;

//Temperature reading setup
int THERMISTOR_PIN = A6;
uint16_t sample[NUMSAMPLES];

float currentTemp; //temperature measurement
boolean showCurrentTemp = true; 
long prevMs = 0; // set up timer

int dispDelay = 1000; // milliseconds to allow set temp interactions without displaying current temperature (simulated multi-threading)
int debouncer = 400; // milliseconds to delay code for debouncing

// Alarm Parameters
boolean soundAlarm = false;
//int melody[] = { NOTE_C4, NOTE_C4, NOTE_C4 };
//int noteDurations[] = { 4, 4, 4};  // note durations: 4 = quarter note, 8 = eighth note, etc.

//PID Variables
//float current_temperature; // temperature measurement
float current_error; //how far form the target temperature we are.
//float target_temperature = 32; //set temperature target (can also be set with the interface. Remember that the steady-state temperature will be 1 deg. less
float old_temp; // Parameter for derivative term
int controlSignal; //Sum of Pterm and (future) Dterm

//PWM setup
int bassinetPin = 9;



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
  pinMode(HOT_LED, OUTPUT); //sets LED pins to output
  pinMode(COLD_LED, OUTPUT); 
  pinMode(leftPin, OUTPUT); 
  pinMode(rightPin, OUTPUT);
  pinMode(THERMISTOR_PIN, INPUT); 
  pinMode(bassinetPin, OUTPUT);
  
  analogReference(EXTERNAL); 
  setPwmFrequency(bassinetPin,1); // Bassinet hums out of hearing range @ 61,250 Hz http://playground.arduino.cc/Code/PwmFrequency

  //7-Seg. Display
  #ifndef __AVR_ATtiny85_
  Serial.println("7 Segment Backpack Test");
  #endif
  matrix.begin(0x70); // begins I2C communication with seven segment display
  matrix.setBrightness(3); // sets brightness, on scale of 0 (dim) to 15 (bright)
  turnOffDisp(); 
}

void loop() {
// Power button code
  int power_current = analogRead(PWR_BUTTON_PIN); // read power button state
  if (power_current >= 1000 && power_last_state <= 50) { // if power button is pressed
    if (sevseg_on){
      turnOffDisp(); // turn off display if it is on
      sevseg_on = false;
    }
    else{
      currentTempUpdate(); // turn on display if it was off
      sevseg_on = true;
    }
  }
  power_last_state = power_current; // refresh button state in memory
  if (sevseg_on) {
    if (showCurrentTemp){
      currentTempUpdate(); // only show current temperature if the display is on and the user is not changing the set temp
    }
 
      // 7-Seg. Display Code
    upTemp = analogRead(UP_BUTTON_PIN); // get signal fom membrane switch (up arrow)
    downTemp = analogRead(DOWN_BUTTON_PIN); //get signal from membrane switch (down arrow)
    if (upTemp >= 1000 && setTemp < maxTemp) {  // condition that membrane switch is high (pressed) and that the desired set temp is less than max
      setTemp = setTemp+1; // increment set temperature by 1
    }
    if (downTemp >= 1000 && setTemp > minTemp){ //condition that membrane switch is high (pressed) and that desired set temp is greater than min
      setTemp = setTemp-1; // increment set temperature by -1
    }
    if (upTemp >= 1000 || downTemp >= 1000){
      setTempUpdate(); // show temp even when already at min/ max
      showCurrentTemp = false; //pause currentTemp updates
      prevMs = millis(); //get time of latest setTemp button press
    }
    unsigned long newMs = millis(); 
    if(newMs - prevMs > dispDelay){ // check if enough time has passed since latest setTemp button press
      showCurrentTemp = true; // if so, allow current temp display again
    }
    PID_loop();
      Serial.print("Target: "); Serial.print(setTemp);
  Serial.print("  Current: "); Serial.print(currentTemp);
  Serial.print("  Error: "); Serial.print(current_error);
  Serial.print("  Control Sig:  "); Serial.println(controlSignal);
  }


// Alarm button code -- WILL ALARM EVEN WHEN DISPLAY IS NOT ON
  int alarm_current = analogRead(ALARM_BUTTON_PIN);  // read alarm button state
  if (alarm_current >= 1000 && alarm_last_state <= 50) { //if alarm button is pressed anew, min threshold 50 for debouncing (despite pull down) 
    digitalWrite(HOT_LED, !digitalRead(HOT_LED));       //toggled the LED state
//    digitalWrite(HOT_LED, HIGH);
    soundAlarm = !soundAlarm; 
  }
  alarm_last_state = alarm_current; // refresh button state in memory
  if(soundAlarm){
//      diffDriveAlarm(3,5); //NOTE: HOLD DOWN PWR/ ALARM BUTTON TO TURN OFF ALARM WHEN STARTED
//      delay(100); 
  }
}

void setTempUpdate() {
  int setTemp_tens = setTemp/10; 
  int setTemp_ones = setTemp%10;
  matrix.writeDigitNum(0, setTemp_tens);
  matrix.writeDigitNum(1, setTemp_ones, false);
  matrix.writeDigitRaw(3,0);
  matrix.writeDisplay(); // refreshes display with new content
  delay(debouncer);
  return;
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
  return;
}

float get_temperature() { //Receive temperature measurement
  uint8_t i;
  float average; 
  for (i=0; i< NUMSAMPLES; i++) {
    sample[i]= analogRead(THERMISTOR_PIN);
    delay(50);
  }
  for (i=0; i< NUMSAMPLES; i++) {
     average += sample[i];
  }
  average /= NUMSAMPLES;
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  Serial.print("Thermistor resistance "); 
  Serial.println(average);
 
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to C
  Serial.print("Temp: ");
  Serial.println(steinhart); 
  return steinhart;
}
void turnOffDisp(){
    for (int i=0; i<=4; i++){
      matrix.writeDigitRaw(i,0); 
    }
    matrix.writeDisplay();
    return;
}

//void medAlarm(){ 
//    for (int thisNote = 0; thisNote < 3; thisNote++) {
//      int noteDuration = 1000 / noteDurations[thisNote];
//      tone(BUZZER, melody[thisNote], noteDuration);
//      // to distinguish the notes, set a minimum time between them.
//      // the note's duration + 30% seems to work well:
//      int pauseBetweenNotes = noteDuration * 1.30;
//      delay(pauseBetweenNotes);
//      // stop the tone playing:
//      noTone(BUZZER);
//    }
//    return;
//  }

void diffDriveAlarm (int repeats, int duration){
  int i, r; 
  for (r=0; r<repeats; r++){
    for (i=0; i<duration; i++){
//      diffDriveTone(2093,1000000);
      diffDriveTone(2270,1000000); // 440 Hz A for 1 sec
    }
    delay(300);
  }
  return;
}
void diffDriveTone (int period, int duration) {
//   NOTE: Buzzer stops making sounds at durations > 1000000 microseconds. 
//  Pulled from https://forums.adafruit.com/viewtopic.php?f=25&t=29920  
    char phase = 0;
    int i;
    
    for (i=0 ; i < duration ; i+=period) {
        digitalWrite( leftPin, phase & 1 ); // bit bang to double volume -- "differential drive"
        phase++;
        digitalWrite( rightPin, phase & 1 );
        delayMicroseconds( period );
    }
    return;
}

void PID_loop() {
  current_error = setTemp - currentTemp; //calculate error
  controlSignal = round(150*current_error+10*(currentTemp-old_temp)); // P + D control. But the D control is set to 0, becuase it doesn't really do anything yet. It's based on temperature change. Need to avg set of temp values to see more change for Dterm to actually be effective.
  if (controlSignal < 0){ //When control signal becomes negative, set it to zero.
    controlSignal = 0; 
  }
  if (controlSignal > 255) { //When control signal exceeds the maximum value, set it to the maximum value 255.
    controlSignal = 255;
  }
  Serial.print("Control: ");
  Serial.println(controlSignal);
  analogWrite(bassinetPin, controlSignal); //Produce PWM at specified control signal cycle.
}

// Frequency Magic --> Controls the frequency of the arduino pin, so that PWM is not audible
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
    TCCR1B = TCCR1B & 0b11111000 | mode;
  }
}


