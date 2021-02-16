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
  
  // set the RTC time and date to the compile time
  RTC.set(compileTime());
  
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
    Serial.print(tm.Hour, DEC);
    Serial.print(':');
    Serial.print(tm.Minute, DEC);
    Serial.print(':');
    Serial.print(tm.Second, DEC);
    Serial.print(" Temperatur: ");
    Serial.print(celcius);
    Serial.println(" Celcius");
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

// function to return the compile date and time as a time_t value
time_t compileTime()
{
    const time_t FUDGE(10);    //fudge factor to allow for upload time, etc. (seconds, YMMV)
    const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char compMon[4], *m;

    strncpy(compMon, compDate, 3);
    compMon[3] = '\0';
    m = strstr(months, compMon);

    tmElements_t tm;
    tm.Month = ((m - months) / 3 + 1);
    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);

    time_t t = makeTime(tm);
    return t + FUDGE;        //add fudge factor to allow for compile time
}
