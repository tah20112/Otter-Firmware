const int PWR_BUTTON_PIN = A3; 
const int ALARM_BUTTON_PIN = A2;
const int HOT_LED = 12; // LED pin for "too hot" alarm
const int COLD_LED = 11; // LED pin for "too cold" alarm
boolean soundAlarm = false;
int alarm_last_state = 0; //last read on alarm button (analog 0 - 1023)
const int leftPin = 9; 
const int rightPin = 13;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  diffDriveAlarm(3,5);
  digitalWrite(HOT_LED, HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:
// Alarm button code -- WILL ALARM EVEN WHEN DISPLAY IS NOT ON
  int alarm_current = analogRead(ALARM_BUTTON_PIN);  // read alarm button state
  if (alarm_current >= 1000 && alarm_last_state <= 50) { //if alarm button is pressed anew, min threshold 50 for debouncing (despite pull down) 
    Serial.println("pressed");
//    digitalWrite(HOT_LED, !digitalRead(HOT_LED));       //toggled the LED state
    digitalWrite(HOT_LED, HIGH); 
    soundAlarm = !soundAlarm; 
  }
  Serial.print("LED");
  Serial.println(digitalRead(HOT_LED)); 
  alarm_last_state = alarm_current; // refresh button state in memory
  if(soundAlarm){
      diffDriveAlarm(3,5); //NOTE: HOLD DOWN PWR/ ALARM BUTTON TO TURN OFF ALARM WHEN STARTED
//      delay(100); 
  }
}
void diffDriveAlarm (int repeats, int duration){
  int i, r; 
  for (r=0; r<repeats; r++){
    for (i=0; i<duration; i++){
      diffDriveTone(2093,1000000);
//      diffDriveTone(2270,1000000); // 440 Hz A for 1 sec
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
