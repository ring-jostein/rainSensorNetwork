#include <DS3232RTC.h>
#include <TimeLib.h>
<<<<<<< Updated upstream

const byte interruptPinRTC = 2;
const byte ledPin = 13;
volatile byte state = LOW;
=======
#include <avr/sleep.h>

const byte interruptPinRTC = 2;
>>>>>>> Stashed changes
tmElements_t tm;

void setup() // put your setup code here, to run once
{
<<<<<<< Updated upstream
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
=======
  pinMode(interruptPinRTC, INPUT_PULLUP);
  Serial.begin(115200);

  //Fjern kommentar og endre verdier for å stille RTC. Kommenter igjen for å hindre RTC fra å bli stilt hver gang du laster opp kode
  
  tm.Hour = 14;
  tm.Minute = 10;
  tm.Second = 50;
  tm.Day = 12;
  tm.Month = 2;
  tm.Year = 2021 - 1970;    //tmElements_t.Year is the offset from 1970
  RTC.write(tm);  //set the RTC from the tm structure

  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);
  
  RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0);  //setter alarm på RTC til å gå av hvert minutt
  RTC.alarmInterrupt(ALARM_2, true);
}

void loop() // put your main code here, to run repeatedly
{
  delay(5000);
  sleepMode();
}

void sleepMode()
{
  Serial.println("Going to sleep...");
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(interruptPinRTC), wakeUp, FALLING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  sleep_cpu();
  
  Serial.println("Waking up...");
  digitalWrite(LED_BUILTIN, HIGH);
  
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
  delay(1000);
}

void alarmRTC() //funksjon som interrupter ved å gi alarm
{
  state = !state;
=======
  
  RTC.alarm(ALARM_2);
}

void wakeUp() //funksjon som interrupter ved å gi alarm
{
  detachInterrupt(digitalPinToInterrupt(interruptPinRTC));
  Serial.println("Interrupted sleep");
  sleep_disable();
>>>>>>> Stashed changes
}
