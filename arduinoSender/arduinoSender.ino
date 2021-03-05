#include <WiFiEspAT.h>
#include <SD.h>
#include <SPI.h>
#include <DS3232RTC.h>
#include <TimeLib.h>
#include <avr/sleep.h>

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); //TXD, RXD
#endif

#define MAX_CLIENTS WIFIESPAT_LINKS_COUNT
#define interruptPinRTC 2
#define interruptPinRainGauge 3
#define chipSelect 10
#define delim1 '.'
#define delim2 ':'
#define delim3 ' '

tmElements_t tm;
File dataFil;

void setup()
{
  digitalWrite(LED_BUILTIN, LOW); //strømsparing
  pinMode(interruptPinRTC, INPUT_PULLUP);
  pinMode(interruptPinRainGauge, INPUT_PULLUP);
  
  // initialize serial for debugging
  Serial.begin(2000000);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);
  
  if (WiFi.status() == WL_NO_MODULE) 
  {
    Serial.println();
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // waiting for connection to Wifi network set with the SetupWiFiConnection sketch
  Serial.println("Waiting for connection to WiFi");
  while (WiFi.status() != WL_CONNECTED) {}
  Serial.println("Connected");

  //Initialiserer SD-kort og sjekker for feil
  Serial.println("Initialiserer SD-kort...");
  if(!SD.begin(chipSelect))
  {
    Serial.println("Initialisering feilet, sjekk SD-kort!");
    while (true);
  }
  else Serial.println("Initialisering ferdig.");
  
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

//funksjon for å konstruere nødvendig data i en String og lagre disse til SD-kort
void dataLogger(bool alarmCheck)
{
  String dataString = "";
  
  RTC.read(tm);
  int t = RTC.temperature();
  int celcius = t / 4.0;

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
  lagreTilSD(dataString);
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
  dataFil = SD.open("datalog.txt", FILE_WRITE);

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

//Henter ut kompileringsdato og tid. Gjør om char strings fra DATE og TIME til INT-verdier som kan gies til tm.
time_t compileTime()
{
  const time_t FUDGE(10);  //definerer tiden det tar å kompilere koden
  const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char compMon[4], *m;
  
  strncpy(compMon, compDate, 3); //kopierer de tre første karakterene fra compDate inn i compMon (måned i trebokstavsforkortelse)
  compMon[3] = '\0';
  m = strstr(months, compMon);  //substring fra punktet stringen compMon finnes i months
  
  //finner heltallet for punktet m starter i months. Ved å dele på tre vil hver måned representeres som et tall fra 0-11. +1 for å få verdier fra 1-12.
  tm.Month = ((m - months) / 3 + 1);  
  
  //konverter substring fra definert punkt til neste "whitespace" til et heltall.
  tm.Day = atoi(compDate + 4);
  tm.Year = atoi(compDate + 7) - 1970;  //trekker fra 1970 fordi år på RTC er antall år etter 1970. Eks. 2021-1970 = 51
  tm.Hour = atoi(compTime);
  tm.Minute = atoi(compTime + 3);
  tm.Second = atoi(compTime + 6);
  
  time_t t = makeTime(tm);  //tid t er alle elementene tm
  return t + FUDGE;  //legger til fudge for å kompensere for tiden det tar å kompilere
}
