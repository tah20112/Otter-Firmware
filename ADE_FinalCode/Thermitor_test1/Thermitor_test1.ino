
#include <Wire.h> //Temperature probe reading library
//#include <OneWire.h> //Temperature probe reading library
//#include <math.h> //Arduino library for # processing

//#define ONE_WIRE_BUS 2
//OneWire oneWire(ONE_WIRE_BUS);
//DallasTemperature sensors(&oneWire);

// which analog pin to connect
#define THERMISTOR_PIN0 A0
#define THERMISTOR_PIN1 A1
#define THERMISTOR_PIN2 A2
#define THERMISTOR_PIN3 A3
#define THERMISTOR_PIN4 A4 
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
 
uint16_t sample0[NUMSAMPLES];
uint16_t sample1[NUMSAMPLES];
uint16_t sample2[NUMSAMPLES];
uint16_t sample3[NUMSAMPLES];
uint16_t sample4[NUMSAMPLES];

//PID Variables
float current_temperature0; // temperature measurement
float current_temperature1; // temperature measurement
float current_temperature2; // temperature measurement
float current_temperature3; // temperature measurement
float current_temperature4; // temperature measurement
float maximum_error; 
float overshoot_error; 

void setup() {
  //PWM setup
  pinMode(THERMISTOR_PIN0, INPUT);
  pinMode(THERMISTOR_PIN1, INPUT);
  pinMode(THERMISTOR_PIN2, INPUT);
  pinMode(THERMISTOR_PIN3, INPUT);
  pinMode(THERMISTOR_PIN4, INPUT);
  analogReference(EXTERNAL);//idk why this is here
  Serial.begin(9600); // open serial port at 100 bps -lets us communicate w arduino

  
}

void loop() {

}

float sense_temperature() { //Receive temperature measurement
  uint8_t i;
  float average0=0;
  float average1=0;
  float average2=0; 
  float average3=0;
  float average4=0;
  
  float temperature0;
  float temperature1;
  float temperature2;
  float temperature3;
  float temperature4;
  
  for (i=0; i< NUMSAMPLES; i++) {
    sample0[i]= analogRead(THERMISTOR_PIN0);
    sample1[i]= analogRead(THERMISTOR_PIN1);
    sample2[i]= analogRead(THERMISTOR_PIN2);
    sample3[i]= analogRead(THERMISTOR_PIN3);
    sample4[i]= analogRead(THERMISTOR_PIN4);
    delay(50);
    for (i=0; i< NUMSAMPLES; i++) {
     average0 += sample0[i];
     average1 += sample1[i];
     average2 += sample2[i];
     average3 += sample3[i];
     average4 += sample4[i];
         }
  }
  temperature0 = calculate_temperature (average0);
  temperature1 = calculate_temperature (average1);
  temperature2 = calculate_temperature (average2);
  temperature3 = calculate_temperature (average3);
  temperature4 = calculate_temperature (average4);

  Serial.print(temperature0);
  Serial.print(temperature1);
  Serial.print(temperature2);
  Serial.print(temperature3);
  Serial.println(temperature4); 
}

float calculate_error (float temperature) {
  maximum_error = 0
  
}

  
float calculate_temperature (float average) { //would be better to input sample arrays
  average /= NUMSAMPLES;
  average = 1023 / average - 1;
  average = SERIESRESISTOR / average;
  
  float steinhart;
  steinhart = average / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert to Cs
  return steinhart;
}

