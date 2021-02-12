#include <DS3232RTC.h>
#include <TimeLib.h>

const byte interruptPinRTC = 2;
const byte ledPin = 13;
volatile byte state = LOW;
tmElements_t tm;

void setup() // put your setup code here, to run once
{
  pinMode(ledPin, OUTPUT);
  pinMode(interruptPinRTC, INPUT_PULLUP);
  Serial.begin(115200);
  
  tm.Hour = 23;
  tm.Minute = 31;
  tm.Second = 55;
  tm.Day = 13;
  tm.Month = 2;
  tm.Year = 2009 - 1970;    //tmElements_t.Year is the offset from 1970
  RTC.write(tm);  //set the RTC from the tm structure

  RTC.squareWave(SQWAVE_NONE);
  
  RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0);  //setter alarm på RTC til å gå av hvert minutt
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_2, true);
  attachInterrupt(digitalPinToInterrupt(interruptPinRTC), alarmRTC, FALLING);
}

void loop() // put your main code here, to run repeatedly
{
  digitalWrite(ledPin, state);
  RTC.read(tm);
  int t = RTC.temperature();
  float celcius = t / 4.0;
  Serial.print(tm.Hour, DEC);
  Serial.print(':');
  Serial.print(tm.Minute, DEC);
  Serial.print(':');
  Serial.print(tm.Second, DEC);
  Serial.print(" Temperatur: ");
  Serial.print(celcius);
  Serial.println(" Celcius");
  delay(1000);
}

void alarmRTC() //funksjon som interrupter ved å gi alarm
{
  state = !state;
}
