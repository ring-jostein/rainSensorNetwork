#include <WiFiEspAT.h>
#include <SD.h>

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); //TXD, RXD
#endif

#define MAX_CLIENTS WIFIESPAT_LINKS_COUNT

// telnet defaults to port 2323
WiFiServer server(2323);
WiFiClient clients[MAX_CLIENTS];
int k = 0;

void setup() 
{
  Serial.begin(115200);   // initialize serial for debugging
  Serial1.begin(9600);    // initialize serial for ESP module
  WiFi.init(&Serial1);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) 
  {
    Serial.println("WiFi shield not present");
    while (true); // don't continue
  }
  else Serial.println("Initialisering ferdig.");
  
  // attempt to connect to WiFi network
  Serial.print("Attempting to establish AP");
  WiFi.beginAP();

  // start listening for clients
  server.begin(MAX_CLIENTS);
}

void loop() {
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
  for (byte i=0; i < MAX_CLIENTS; i++) 
  {
    if (clients[i] && clients[i].available() > 0) 
    {
      // read bytes from a client
      byte buffer[80];
      int count = clients[i].read(buffer, 80);
      Serial.write(buffer, count); //kun for testing

      lagreTilSD(buffer, count);
      
      //print confirmation
      clients[i].println("OK");
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
}

void lagreTilSD(byte buffer[80], int count)
{
  
}
