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
#define AT_BAUD_RATE 9600
#else
#define AT_BAUD_RATE 115200
#endif

#define MAX_CLIENTS WIFIESPAT_LINKS_COUNT
#define chipSelect 10

WiFiServer server(2323);

WiFiClient clients[MAX_CLIENTS];

File dataFil;

void setup()
{
  digitalWrite(LED_BUILTIN, LOW); //strømsparing
  
  // initialize serial for debugging
  Serial.begin(115200);
  while (!Serial);
  
  // initialize serial for ESP module
  Serial1.begin(AT_BAUD_RATE);
  // initialize ESP module
  WiFi.init(&Serial1);

  if (WiFi.status() == WL_NO_MODULE) 
  {
    Serial.println();
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
    

  // waiting for connection to Wifi network set with the SetupWiFiConnection sketch
  Serial.println("Waiting for connection to WiFi");
  while (WiFi.status() != WL_CONNECTED) 
    {
      delay(1000);
      Serial.print('.');
    }
  }
  Serial.println("Connected");

  // start listening for clients
  server.begin(MAX_CLIENTS);
  Serial.print("Chat server address:");
  Serial.println(WiFi.localIP());

  //Initialiserer SD-kort og sjekker for feil
  //Serial.println("Initialiserer SD-kort...");
  if(!SD.begin(chipSelect))
  {
    //Serial.println("Initialisering feilet, sjekk SD-kort!");
    while (true);
  }
  //else Serial.println("Initialisering ferdig.");

}
void loop()
{
  // check for any new client connecting, and say hello (before any incoming data)
  WiFiClient newClient = server.accept();
  if (newClient) {
    for (byte i=0; i < MAX_CLIENTS; i++) {
      if (!clients[i]) {
        Serial.print("We have a new client #");
        Serial.println(i);
        newClient.print("Hello, client number: ");
        newClient.println(i);
        // Once we "accept", the client is no longer tracked by EthernetServer
        // so we must store it into our list of clients
        clients[i] = newClient;
        break;
      }
    }
  }

  // check for incoming data from all clients
  for (byte i=0; i < MAX_CLIENTS; i++) {
    if (clients[i] && clients[i].available() > 0) {
      // read bytes from a client
      byte buffer[80];
      int count = clients[i].read(buffer, 80);
      // write the bytes to all other connected clients
      for (byte j=0; j < MAX_CLIENTS; j++) {
        if (j != i && clients[j].connected()) {
          clients[j].write(buffer, count);
        }
      }
    }
  }

  // stop any clients which disconnect
  for (byte i=0; i < MAX_CLIENTS; i++) {
    if (clients[i] && !clients[i].connected()) {
      Serial.print("disconnect client #");
      Serial.println(i);
      clients[i].stop();
    }
  }
  
  //dataLogger();
  //lesFraSD();
  //slettData();
  //sendData();
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
  }
  else regn = '1';

  sprintf(dataString, "%.2d.%.2d.%d %.2d:%.2d:%.2d %i %c", day(t), month(t), year(t), hour(t), minute(t), second(t), celcius, regn);
  //sprintf(dataString, "%.2d:%.2d:%.2d %.2d %s %d %c", hour(t), minute(t), second(t), day(t), monthShortStr(month(t)), year(t), regn);
  Serial.println(dataString); //kun for testing
  
  //lagrer data
  lagreTilSD(dataString);
}

void lagreTilSD(char dataString[])
{
  dataFil = SD.open("datalog.txt", FILE_WRITE);

  if(dataFil)
  {
    dataFil.println(dataString);
    dataFil.close();
  }
  else Serial.println("Feil: Kan ikke åpne datalog.txt fra SD-kort");
}

//testfunksjon for å se at det er data på SD-kortet
void lesFraSD()
{
  dataFil = SD.open("datalog.txt");

  if(dataFil)
  {
    Serial.println("datalog.txt: ");
    while(dataFil.available())
    {
      Serial.write(dataFil.read());
    }
    dataFil.close();
  }
  else Serial.println("Feil: Kan ikke åpne datalog.txt fra SD-kort");
}

void slettData()
{
  SD.remove("datalog.txt");
  if(SD.exists("datalog.txt")) { Serial.println("The file still exists..."); }
  else Serial.println("File deleted");
}
