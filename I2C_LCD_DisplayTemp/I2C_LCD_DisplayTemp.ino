// include the library code
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

int tim = 2000; //the value of delay time for display
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// which analog pin to connect
#define THERMISTORPIN1 A0
#define THERMISTORPIN2 A1
#define THERMISTORPIN3 A2         
// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000      
// temp. for nominal resistance (almost always 25 C)
#define TEMPERATURENOMINAL 25   
// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 3
// The beta coefficient of the thermistor (usually 3000-4000)
#define BCOEFFICIENT 3950
// the value of the 'other' resistor
#define SERIESRESISTOR 10000    
 
uint16_t samples[NUMSAMPLES];
uint16_t num_thermistors = 3; 

void setup()
{
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight

  Serial.begin(9600);
  // connect AREF to 3.3V and use that as VCC, less noisy!
  analogReference(EXTERNAL);
}

void loop(void) {
  uint8_t i;
  float average;
 
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES*num_thermistors; i+=num_thermistors) {
   samples[i] = analogRead(THERMISTORPIN1);
   samples[i+1]= analogRead(THERMISTORPIN2);
   samples[i+2]= analogRead(THERMISTORPIN3);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES*num_thermistors; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES*num_thermistors;
 
  Serial.print("Average analog reading "); 
  Serial.println(average);
 
  // convert the value to resistance
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
 
  Serial.print("Temperature "); 
  Serial.print(steinhart);
  Serial.println(" *C");

  lcd.setCursor(4,0); // set the cursor to column 15, line 0
  lcd.print("Avg Temp");
  lcd.setCursor(5,1); // set the cursor to column 15, line 1
  lcd.print(steinhart); // Print a message to the LCD.
  lcd.print(" *C");
  
  delay(tim); 
  lcd.clear(); //Clears the LCD screen and positions the cursor in the upper-left corner.
}
