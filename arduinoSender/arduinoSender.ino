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
  RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0);  //setter alarm på RTC til å gå av hvert minutt
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_2, true);
  attachInterrupt(digitalPinToInterrupt(interruptPinRTC), alarmRTC, FALLING);

  tmElements_t tm;
  tm.Hour = 23;             //set the tm structure to 23h31m30s on 13Feb2009
  tm.Minute = 31;
  tm.Second = 30;
  tm.Day = 13;
  tm.Month = 2;
  tm.Year = 2009 - 1970;    //tmElements_t.Year is the offset from 1970
  RTC.write(tm);            //set the RTC from the tm structure
}

void loop() // put your main code here, to run repeatedly
{
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
  digitalWrite(ledPin, state);
  delay(1000);
}

void alarmRTC() //funksjon som interrupter ved å gi alarm
{
  state = !state;
  RTC.alarm(ALARM_2);
}
