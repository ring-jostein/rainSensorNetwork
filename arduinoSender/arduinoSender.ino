<<<<<<< Updated upstream
#include <DS3232RTC.h>
#include <TimeLib.h>
#include <avr/sleep.h>

const byte interruptPinRTC = 2;
const byte interruptPinRainGauge = 3;
tmElements_t tm;

void setup()
{
  digitalWrite(LED_BUILTIN, LOW); //strømsparing
  pinMode(interruptPinRTC, INPUT_PULLUP);
  pinMode(interruptPinRainGauge, INPUT_PULLUP);
  Serial.begin(2000000);
  
  //stiller RTC til dato og tid for kompilering
  RTC.set(compileTime());
  
  //Blokk som setter alarmer til kjente verdier
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  //setter alarm på RTC til å gå av hvert minutt og aktiverer pin for interrupt-signal fra RTC
  RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0);  //gjøres om til hvert 30. minutt ved reell bruk
  RTC.alarmInterrupt(ALARM_2, true);
}

void loop()
{
  sleepMode();
  dataLogger(RTC.checkAlarm(ALARM_2));
  //sendData();
}

//setter Arduino i sleepmode og aktiverer interrupt-pinner som vekker den
void sleepMode()
{
  Serial.println("Going to sleep...");  //kun for testing
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(interruptPinRTC), wakeUpAlarm, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPinRainGauge), wakeUpRain, FALLING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_cpu();
  Serial.println("Waking up...");  //kun for testing
}

void dataLogger(bool alarmCheck)
{
  String dataString = "";
  int t = 0;
  int celcius = 0;
  char delim1 = '.';
  char delim2 = ':';
  char delim3 = ' ';
  
  RTC.read(tm);
  t = RTC.temperature();
  celcius = t / 4.0;

  //Konstruerer og formaterer string av data
  for(int i = 1; i < 7; i++)
  {
    switch(i)
    {
      case 1:
      dataString += format00(tm.Day, delim1);
      break;

      case 2:
      dataString += format00(tm.Month, delim1);
      break;

      case 3:
      dataString += String(tm.Year + 1970, DEC);
      dataString += delim3;
      break;

      case 4:
      dataString += format00(tm.Hour, delim2);
      break;

      case 5:
      dataString += format00(tm.Minute, delim2);
      break;

      case 6:
      dataString += format00(tm.Second, delim3);
      break;
    }
  }

  dataString += "Temperatur: ";
  dataString += String(celcius, DEC);
  dataString += "C ";

  //sjekker om det er registrert nedbør eller ikke
  if(alarmCheck) 
  {
    dataString += "Nedbør: 0";
    RTC.clearAlarm(ALARM_2);
  }
  else dataString += "Nedbør: 1";

  //lagrer data
  Serial.println(dataString);  //kun for testing
  //writeToSD()
}

String format00(int verdi, char delim)  //formaterer tall under 10 til å vises som 0X
{
  String string = "";
  
  if(verdi < 10)
  {
    string = '0';
    string += String(verdi, DEC);
    string += delim;
  }
  else 
  {
    string += String(verdi, DEC);
    string += delim;
  }

  return string;
}

void sendData()
{
  
}

void wakeUpAlarm()  //ISR for RTC
{
  detachInterrupt(digitalPinToInterrupt(interruptPinRTC));
  sleep_disable();
}

void wakeUpRain()  //ISR for nedbørsmåler
{
  detachInterrupt(digitalPinToInterrupt(interruptPinRainGauge));
  sleep_disable();
}

//funksjon som gir tilbake dato og tid for kompilering
time_t compileTime()
{
    const time_t FUDGE(10);    //fudge factor to allow for upload time, etc. (seconds, YMMV)
    const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char compMon[4], *m;

    strncpy(compMon, compDate, 3);
    compMon[3] = '\0';
    m = strstr(months, compMon);

    tm.Month = ((m - months) / 3 + 1);
    tm.Day = atoi(compDate + 4);
    tm.Year = atoi(compDate + 7) - 1970;
    tm.Hour = atoi(compTime);
    tm.Minute = atoi(compTime + 3);
    tm.Second = atoi(compTime + 6);

    time_t t = makeTime(tm);
    return t + FUDGE;        //add fudge factor to allow for compile time
}
=======
#include <DS3232RTC.h>
#include <TimeLib.h>
#include <avr/sleep.h>
#include <PrintEx.h> 

const byte interruptPinRTC = 2;
const byte interruptPinRainGauge = 3;
tmElements_t tm;

PrintEx print = Serial;
String timeStamps[99];
int timeStampCounter = 0;
String tempTimeStamp = "";

void setup() // put your setup code here, to run once
{
  
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

// set the RTC time and date to the compile time
    RTC.set(compileTime());

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
  sleepMode();
}

void sleepMode()
{
  Serial.println("Going to sleep...");
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(interruptPinRTC), wakeUp1, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPinRainGauge), wakeUp2, FALLING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  digitalWrite(LED_BUILTIN, LOW);
  sleep_cpu();
  Serial.println("Waking up...");
  digitalWrite(LED_BUILTIN, HIGH);
  
  RTC.read(tm);
  int t = RTC.temperature();
  float celcius = t / 4.0;
  /*
  Serial.print(tm.Hour, DEC);
  Serial.print(':');
  Serial.print(tm.Minute, DEC);
  Serial.print(':');
  Serial.println(tm.Second, DEC);
  */
  //print.printf( "%02d:%05d:%06d", tm.Hour, tm.Minute, tm.Second);



  //timeStamps[timeStampCounter] = makeTimeIntoString(tm.Hour, tm.Minute, tm.Second);
  //timeStamps[timeStampCounter] = 
  String temp = printf( "%02d:%02d:%02d Halaisen %d", tm.Hour, tm.Minute, tm.Second, timeStampCounter);
  Serial.print(temp);
  timeStampCounter++;
/*
  for(int i = 0; i < 50; i++)
  {
    Serial.println(timeStamps[i]);  
  }
 */
  Serial.print(" Temperatur: ");
  Serial.print(celcius);
  Serial.println(" Celcius");
  
  RTC.alarm(ALARM_2);
}


void wakeUp1() //funksjon som interrupter ved å gi alarm
{
  detachInterrupt(digitalPinToInterrupt(interruptPinRTC));
  Serial.println("Interrupted sleep from alarm.");
  sleep_disable();
}

void wakeUp2()
{
  detachInterrupt(digitalPinToInterrupt(interruptPinRainGauge));
  Serial.println("Interrupted sleep from rain gauge.");
  sleep_disable();
}

time_t compileTime()
{
    const time_t FUDGE(10);    //fudge faktor som tillater å laste opp tid (seconds, YMMV)
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

String makeTimeIntoString(int initHour, int initMinute, int initSecond)
{
  int timeHour = initHour;
  int timeMinute = initMinute;
  int timeSecond = initSecond;
  String timeTotal = "";

  timeTotal = timeHour;
  timeTotal += ":";
  timeTotal += timeMinute;
  timeTotal += ":";
  timeTotal += timeSecond;

  return timeTotal;
}
>>>>>>> Stashed changes
