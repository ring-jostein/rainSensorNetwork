#include <DS3232RTC.h>
#include <TimeLib.h>
#include <avr/sleep.h>

const byte interruptPinRTC = 2;
const byte interruptPinRainGauge = 3;
tmElements_t tm;
int t = 0;

void setup() // put your setup code here, to run once
{
  digitalWrite(LED_BUILTIN, LOW); //strømsparing
  pinMode(interruptPinRTC, INPUT_PULLUP);
  pinMode(interruptPinRainGauge, INPUT_PULLUP);
  Serial.begin(2000000);

  //Fjern kommentar og endre verdier for å stille RTC. Kommenter igjen for å hindre RTC fra å bli stilt hver gang du laster opp kode
  /*
  tm.Hour = 14;
  tm.Minute = 47;
  tm.Second = 30;
  tm.Day = 12;
  tm.Month = 2;
  tm.Year = 2021 - 1970;    //tmElements_t.Year is the offset from 1970
  RTC.write(tm);  //set the RTC from the tm structure
  */
  
  //Blokk som setter alarmer til kjente verdier
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0);  //setter alarm på RTC til å gå av hvert minutt
  RTC.alarmInterrupt(ALARM_2, true);            //aktiverer alarm slik at INT flagger når alarm går av
}

void loop() // put your main code here, to run repeatedly
{
  sleepMode();
}

void sleepMode()
{
  Serial.println("Going to sleep...");
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(interruptPinRTC), wakeUp1, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPinRainGauge), wakeUp2, FALLING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_cpu();
  
  Serial.println("Waking up...");
  RTC.read(tm);
  t = RTC.temperature();
  float celcius = t / 4.0;

  if (RTC.checkAlarm(ALARM_2)) //sjekker for om Arduino våknet av alarm
  {
    Serial.println("Interrupted sleep from alarm.");
    Serial.print(tm.Hour, DEC);
    Serial.print(':');
    Serial.print(tm.Minute, DEC);
    Serial.print(':');
    Serial.print(tm.Second, DEC);
    Serial.print(" Temperatur: ");
    Serial.print(celcius);
    Serial.println(" Celcius");
    RTC.clearAlarm(ALARM_2);
  }
  
  else //Arduino vekket av nedbørsmåling
  {
    Serial.println("Interrupted sleep from rain gauge.");
    Serial.println("Nedbørsmåling");
  }
}


void wakeUp1() //funksjon som interrupter ved å gi alarm
{
  detachInterrupt(digitalPinToInterrupt(interruptPinRTC));
  sleep_disable();
}

void wakeUp2()
{
  detachInterrupt(digitalPinToInterrupt(interruptPinRainGauge));
  sleep_disable();
}
