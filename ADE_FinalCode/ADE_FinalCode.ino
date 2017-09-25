// Code architecture for PID Loop
// Uses waterproof sensor

//#include <DallasTemperature.h> //Temperature probe processing library
#include <Wire.h> //Temperature probe reading library
//#include <OneWire.h> //Temperature probe reading library
//#include <math.h> //Arduino library for # processing

//#define ONE_WIRE_BUS 2
//OneWire oneWire(ONE_WIRE_BUS);
//DallasTemperature sensors(&oneWire);

// which analog pin to connect
#define THERMISTOR_PIN A3 
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000    
 
uint16_t sample[NUMSAMPLES];

//PID Variables
float current_temperature; // temperature measurement
float current_error; //how far form the target temperature we are.
float target_temperature = 32; //set temperature target (can also be set with the interface. Remember that the steady-state temperature will be 1 deg. less
float old_temp; // Parameter for derivative term
int controlSignal; //Sum of Pterm and (future) Dterm

//PWM setup
int bassinetPin = 9;

void setup() {
  //PWM setup
  pinMode(bassinetPin, OUTPUT); // Sets the digital pin as output
  pinMode(THERMISTOR_PIN, INPUT);
  analogReference(EXTERNAL);
  Serial.begin(9600); // open serial port at 100 bps
  setPwmFrequency(bassinetPin,1); // Bassinet hums out of hearing range @ 61,250 Hz http://playground.arduino.cc/Code/PwmFrequency
//  sensors.begin();
  
}

void loop() {
  old_temp = current_temperature; //Get old_temp. before taking new temp. measurement for D term
  current_temperature = get_temperature(); //Get current temperature
  PID_loop(); //Enter the PID loop
  Serial.print("Target: "); Serial.print(target_temperature);
  Serial.print("  Current: "); Serial.print(current_temperature);
  Serial.print("  Error: "); Serial.print(current_error);
  Serial.print("  Control Sig:  "); Serial.println(controlSignal);
}

void PID_loop() {
  current_error = target_temperature - current_temperature; //calculate error
  controlSignal = round(150*current_error+1.5*(current_temperature-old_temp)); // P + D control. But the D control is set to 0, becuase it doesn't really do anything yet. It's based on temperature change. Need to avg set of temp values to see more change for Dterm to actually be effective.
  if (controlSignal < 0){ //When control signal becomes negative, set it to zero.
    controlSignal = 0; 
  }
  if (controlSignal > 255) { //When control signal exceeds the maximum value, set it to the maximum value 255.
    controlSignal = 255;
  }
  controlSignal = 255;
  analogWrite(bassinetPin, controlSignal); //Produce PWM at specified control signal cycle.
}

float get_temperature() { //Receive temperature measurement
  uint8_t i;
  float average=0; 
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
  steinhart -= 273.15;                         // convert to Cs
  return steinhart;
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
