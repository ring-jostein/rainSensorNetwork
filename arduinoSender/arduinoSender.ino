#include <WiFiEspAT.h>
#include <SD.h>
#include <SPI.h>
#include <DS3232RTC.h>
#include <TimeLib.h>
#include <avr/sleep.h>

// Emulerer Serial1 på pinner 6/7 hvis ikke tilstede
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); //TXD, RXD
#endif

#define interruptPinRTC 2
#define interruptPinRainGauge 3
#define chipSelect 10
#define server "192.168.4.1"  //Endres til korrekt IP for mottaker
#define fileName "datalog.txt"  //Filnavn som data blir lagret til
#define errorSD "Feil: Kan ikke åpne datalog.txt fra SD-kort"  //Definert her ettersom feilmeldingen brukes flere steder i koden

tmElements_t tm;
time_t start;
WiFiClient client;

void setup()
{
  digitalWrite(LED_BUILTIN, LOW); //strømsparing
  pinMode(interruptPinRTC, INPUT_PULLUP);
  pinMode(interruptPinRainGauge, INPUT_PULLUP);
  
  // Serial for debugging
  Serial.begin(2000000);
  // Serial for ESP modul
  Serial1.begin(9600);
  // Initialiser WiFi modul
  WiFi.init(&Serial1);

  //  Bekrefter kommunikasjon med WiFi-modul
  if (WiFi.status() == WL_NO_MODULE) 
  {
    Serial.println(F("Feil: Ingen tilkobling til WiFi-modul"));
    while (true);  // Ikke fortsett
  }

  //  Kobler opp til mottakers aksesspunkt. SSID og passord allerede konfigurert på WiFi-modul
  Serial.print(F("Kobler opp til AP"));
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(F("."));
    delay(1000);
  }
  Serial.println(F("Fullført"));

  //  Initialiserer SD-kort og sjekker for feil
  if(!SD.begin(chipSelect)) 
  {
    Serial.println(F("Feil: Finner ikke SD-kort"));
    while (true);  // Ikke fortsett
  }
  Serial.println(F("SD-kort initialisert"));
 
  RTC.set(compileTime());  //  Stiller klokke til kompileringsdato og tidspunkt, kan kommenteres ut når klokken er stilt.
  start = RTC.get();  //  Henter starttidspunkt fra klokken
  
  //  Blokk som setter alarmer til kjente verdier
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  //  setter alarm på RTC til å gå av hvert minutt og aktiverer pin for interrupt-signal fra RTC
  RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0);  //gjøres om til hvert 30. minutt ved reell bruk
  RTC.alarmInterrupt(ALARM_2, true);
}

void loop()
{
  sleepMode();
  dataLogger(RTC.checkAlarm(ALARM_2));
  lesFraSD();  //kun for testing, verifiserer at data blir skrevet til SD-kort
  sendData();
}

//setter Arduino i sleepmode og aktiverer interrupt-pinner som vekker den
void sleepMode()
{
  Serial.println(F("Aktiverer sleep mode"));
  sleep_enable();
  attachInterrupt(digitalPinToInterrupt(interruptPinRTC), wakeUpAlarm, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptPinRainGauge), wakeUpRain, FALLING);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_cpu();
}

//  funksjon for å konstruere streng av data og lagre disse til SD-kort
void dataLogger(bool alarmCheck)
{
  char dataString[40];
  time_t t = RTC.get();
  char regn;
  int celcius = RTC.temperature() / 4.0;

  // Sjekker om alarm vekket systemet
  if (alarmCheck)
  {
    regn = '0';
    RTC.clearAlarm(ALARM_2);  // Resett alarm
  }
  else regn = '1';

  sprintf(dataString, "%.2d.%.2d.%d %.2d:%.2d:%.2d %i %c", day(t), month(t), year(t), hour(t), minute(t), second(t), celcius, regn);
  Serial.println(dataString);
  
  //lagrer data
  lagreTilSD(dataString);
}

//  Funksjon for å lagre data til SD-kort
void lagreTilSD(char dataString[])
{
  File dataFil = SD.open(fileName, FILE_WRITE);

  if(dataFil) // Sjekker om SD-kort er tilgjengelig
  {
    dataFil.println(dataString);  // Skriver data til SD-kort
    dataFil.close();  // Lukk fil
  }
  else Serial.println(F(errorSD));
}

//Funksjon for å sende data til mottaker
void sendData()
{
  bool skalJegSende = timer();  //Sjekker om det har gått tilstrekkelig tid for å sende
  if(skalJegSende)
  {
    File dataFil = SD.open(fileName);
    if(client.connect(server, 2323))  //  Kobler til server
    {
      if (dataFil)  // Sjekker om SD-kort er tilgjengelig
      {
        while(dataFil.available())  // Går så lenge det er data på SD-kortet
        {
          client.write(dataFil.read());  // Sender data til server
        }
        dataFil.close();   //Lukk fil
        SD.remove(fileName);  //Slett fil
        client.stop();  //Lukk tilkobling til server
      }
    }
    else Serial.println(F(errorSD));
  }  
}

// Timer for å sjekke sendeintervallet
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
  File dataFil = SD.open(fileName);

  if(dataFil)
  {
    Serial.println("datalog.txt: ");
    while(dataFil.available())
    {
      Serial.write(dataFil.read());
    }
    dataFil.close();
  }
  else Serial.println(F(errorSD));
}

//  Henter ut kompileringsdato og tid. Gjør om char strings fra DATE og TIME til heltallsverdier som kan gies til tm.
time_t compileTime()
{
  const time_t FUDGE(10);  //definerer tiden det tar å kompilere koden
  const char *compDate = __DATE__, *compTime = __TIME__, *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  char compMon[4], *m;
  
  strncpy(compMon, compDate, 3); //kopierer de tre første karakterene fra compDate inn i compMon (måned i trebokstavsforkortelse)
  compMon[3] = '\0';
  m = strstr(months, compMon);  //substring fra punktet stringen compMon finnes i months
  
  //finner heltallet for punktet m starter i months. Ved å dele på tre vil hver måned representeres som et heltall fra 0-11. +1 for å få verdier fra 1-12.
  tm.Month = ((m - months) / 3 + 1);  
  
  //konverter substring fra definert punkt til et heltall til neste "whitespace".
  tm.Day = atoi(compDate + 4);
  tm.Year = atoi(compDate + 7) - 1970;  //trekker fra 1970 fordi år på RTC er antall år etter 1970. Eks. 2021-1970 = 51
  tm.Hour = atoi(compTime);
  tm.Minute = atoi(compTime + 3);
  tm.Second = atoi(compTime + 6);
  
  time_t t = makeTime(tm);  //tid t er alle elementene tm
  return t + FUDGE;  //legger til fudge for å kompensere for tiden det tar å kompilere
}
