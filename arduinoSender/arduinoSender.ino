#include <SD.h>
#include <SPI.h>
#include <DS3232RTC.h>
#include <TimeLib.h>
#include <avr/sleep.h>
#include <ESP8266Wifi.h>

const byte interruptPinRTC = 2;
const byte interruptPinRainGauge = 3;
const int chipSelect = 10; //endre til rett verdi basert på type SD-kort shield
tmElements_t tm;

void setup()
{
  digitalWrite(LED_BUILTIN, LOW); //strømsparing
  pinMode(interruptPinRTC, INPUT_PULLUP);
  pinMode(interruptPinRainGauge, INPUT_PULLUP);
  Serial.begin(2000000);
  while(!Serial);

  /*
  //Initialiserer SD-kort og sjekker for feil
  Serial.println("Initialiserer SD-kort...");
  if(!SD.begin(chipSelect))
  {
    Serial.println("Initialisering feilet, sjekk SD-kort!");
    while (true);
  }
  else Serial.println("Initialisering ferdig.");
  */
  
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
  Serial.println();
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
  Serial.println();
  //lagreTilSD(dataString);
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

void lagreTilSD(String dataString)
{
    File dataFil = SD.open("datalog.txt", FILE_WRITE);

    if(dataFil)
    {
      dataFil.println(dataString);
      dataFil.close();
    }
    else Serial.println("Feil: Kan ikke åpne datalog.txt fra SD-kort");
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
