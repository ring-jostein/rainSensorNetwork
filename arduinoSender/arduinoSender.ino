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

#define interruptPinRTC 2
#define interruptPinRainGauge 3
#define chipSelect 10
#define server "10.0.0.10"
#define fileName "datalog.txt"
#define errorSD "Feil: Kan ikke åpne datalog.txt fra SD-kort"
#define sleepMsg "Going to sleep..."
#define errorServer "Could not establish connection"

tmElements_t tm;
time_t start;
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
  
  if (WiFi.status() == WL_NO_MODULE) {while (true);}  // don't continue
  
  while (WiFi.status() != WL_CONNECTED) {}

  //Initialiserer SD-kort og sjekker for feil
  if(!SD.begin(chipSelect)) {while (true);}  // don't continue

  start = RTC.get();
  
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
  //lesFraSD();
}

//setter Arduino i sleepmode og aktiverer interrupt-pinner som vekker den
void sleepMode()
{
  Serial.println(sleepMsg);  //kun for testing
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(interruptPinRTC), wakeUpAlarm, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPinRainGauge), wakeUpRain, FALLING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_cpu();
}

//funksjon for å konstruere nødvendig data i en String og lagre disse til SD-kort
void dataLogger(bool alarmCheck)
{
  char dataString[40];
  time_t t = RTC.get();
  char regn;
  int celcius = RTC.temperature() / 4.0;

  if (alarmCheck)
  {
    regn = '0';
    RTC.clearAlarm(ALARM_2);
  }
  else regn = '1';

  sprintf(dataString, "%.2d.%.2d.%d %.2d:%.2d:%.2d %i %c", day(t), month(t), year(t), hour(t), minute(t), second(t), celcius, regn);
  Serial.println(dataString);
  
  //lagrer data
  //lagreTilSD(dataString);
  sendData(dataString);
}

void lagreTilSD(char dataString[])
{
  dataFil = SD.open(fileName, FILE_WRITE);

  if(dataFil)
  {
    dataFil.println(dataString);
    dataFil.close();
  }
}

void sendData(char dataString[])
{
  bool skalJegSende = timer();
  if(skalJegSende)
  {
    WiFiClient client;
    if(client.connect(server, 2323))
    {
      delay(1000);
      client.println(dataString);
    }
    else Serial.println(errorServer);
  }  
}
/*
void sendData()
{
  bool skalJegSende = timer();
  if(skalJegSende)
  {
    WiFiClient client;
    if(client.connect(server, 2323))
    {
      dataFil = SD.open(fileName);
      if(dataFil)
      {
        while(dataFil.available())
        {
          byte buffer[40];
          int count = dataFil.read(buffer, 80);
          client.write(buffer, count);
        }
        dataFil.close();
        SD.remove(fileName);
      }
    }
  }  
}
*/

bool timer()
{
  bool check = false;
  time_t naa = RTC.get();
  Serial.println(naa-start);
  if(naa - start >= 15) //endres til 300 for å få hvert 5. minutt
  {
    check = true;
    start = RTC.get();
  }
  return check;
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

//testfunksjon for å se at det er data på SD-kortet
void lesFraSD()
{
  dataFil = SD.open("datalog.txt");

  if(dataFil)
  {
    //Serial.println("datalog.txt: ");
    while(dataFil.available())
    {
      Serial.write(dataFil.read());
    }
    dataFil.close();
  }
}
