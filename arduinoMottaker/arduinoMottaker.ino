#include <WiFiEspAT.h>
#include <SD.h>

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); //TXD, RXD
#endif

#define maxClients 4  //ESP-01 håndterer ikke fler enn 4 klienter
#define chipSelect 10  //pinne for kommunikasjon med SD-kort
#define fileName "datalog.txt"
#define errorSD "Feil: Kan ikke åpne datalog.txt fra SD-kort"

WiFiServer server(2323);
WiFiClient clients[maxClients];

void setup() 
{
  Serial.begin(115200);   // initialiserer serial for debugging
  Serial1.begin(9600);    // initialiserer serial for WiFi
  WiFi.init(&Serial1);    // initialiserer WiFi-modul

  // Tester kommunikasjon med WiFi-modul
  if (WiFi.status() == WL_NO_SHIELD) 
  {
    Serial.println(F("Feil: Finner ikke WiFi-modul"));
    while (true); // don't continue
  }

  //Initialiserer SD-kort og sjekker for feil
  if(!SD.begin(chipSelect))
  {
    Serial.println(F("Feil: Finner ikke SD-kort"));
    while (true); // don't continue
  }
  
  // Starter aksesspunkt
  WiFi.beginAP();

  // starter server
  server.begin(maxClients);
}

void loop() {
  // Leter etter nye klienter og legger dem til et klientarray
  WiFiClient newClient = server.accept();
  if (newClient) 
  {
    for (byte i=0; i < maxClients; i++) 
    {
      if (!clients[i]) 
      {
        Serial.print(F("Ny klient #"));
        Serial.println(i);
        newClient.print(F("Hello, client number: "));
        newClient.println(i);
        clients[i] = newClient;
        break;
      }
    }
  }

  // Sjekker innkommende data fra klienter
  for (byte i=0; i < maxClients; i++) 
  {
    if (clients[i] && clients[i].available() > 0) 
    {
      // les bytes fra klient
      byte buffer[80];
      int count = clients[i].read(buffer, 80);
      Serial.write(buffer, count);  //kun for testing
      Serial.println();  //kun for testing

      lagreTilSD(buffer, count);
      //lesFraSD();
      
      //print bekreftelse
      clients[i].println(F("OK"));
    }
  }

  // Fjerner klienter som kobler av fra array
  for (byte i=0; i < maxClients; i++) {
    if (clients[i] && !clients[i].connected()) {
      Serial.print(F("Kobler fra klient #"));
      Serial.println(i);
      clients[i].stop();
    }
  }
}

void lagreTilSD(byte buffer[80], int count)
{
  File dataFil = SD.open(fileName, FILE_WRITE);

  if(dataFil)
  {
    dataFil.write(buffer, count);
    dataFil.close();
  }
  else Serial.println(F(errorSD));
}

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
