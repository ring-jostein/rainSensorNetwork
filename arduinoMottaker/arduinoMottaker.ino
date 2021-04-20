#include <SPI.h>
#include <WiFiNINA.h>
#include <SD.h>
#include <Ethernet.h>
#include "secrets.h"

#define chipSelect 4  // pinne 4 for kommunikasjon med SD-kort for MKR, 10 for Uno
#define fileName "datalog.txt"  // Filnavn for data på SD-kort
#define errorSD "Feil: Kan ikke åpne datalog.txt fra SD-kort"  // Felles feilmelding flere steder i koden

const char ssid[] = SECRET_SSID;
const char pass[] = SECRET_PASS;

int status = WL_IDLE_STATUS;
WiFiServer server(2323);
EthernetClient ethClient;

// Sett MAC- og IP-adresse for ethernetkontroller under. Gateway og subnet er valgfri
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 1, 177);
IPAddress myDns(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

void setup() 
{
  Serial.begin(2000000);   // initialiserer serial for debugging

  // Initialiserer Ethernet modul
  Ethernet.begin(mac, ip, myDns, gateway, subnet);

  //  Tester kommunikasjon med Ethernet Modul
  if (Ethernet.hardwareStatus() == EthernetNoHardware) 
  {
    Serial.println(F("Feil: Finner ikke Ethernet-modul"));
    while (true);  //Ikke fortsett
  }
  if (Ethernet.linkStatus() == LinkOFF) 
  {
    Serial.println(F("Ethernetkabel er ikke tilkoblet"));
    while (true);  //Ikke fortsett
  }
  
  // Tester kommunikasjon med WiFi-modul
  if (WiFi.status() == WL_NO_SHIELD) 
  {
    Serial.println(F("Feil: Finner ikke WiFi-modul"));
    while (true); //Ikke fortsett
  }

  //Initialiserer SD-kort og sjekker for feil
  if(!SD.begin(chipSelect))
  {
    Serial.println(F("Feil: Finner ikke SD-kort"));
    while (true); //Ikke fortsett
  }
  
  // Starter aksesspunkt
  Serial.print(F("Oppretter aksesspunkt med SSID: "));
  Serial.println(ssid);

  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING)
  {
    Serial.println(F("Feil: Kunne ikke opprette aksesspunkt"));
    while (true);  //Ikke fortsett
  }
  
  //starter server
  server.begin();
  Serial.println(F("Server opprettet"));

  //print status
  printWiFiStatus();
}

void loop() 
{
  WiFiClient client = server.available();  // Ny klient basert på innkommende data

  int noCharCount = 0; //Teller runder koden går uten innkommende data
  if (client)
  {
    while (client.connected())
    {
      while (client.available())
      {
        byte buffer[160];  // Buffer for data
        int count = client.read(buffer, 160); // Skriver data til buffer og teller lengden
        Serial.write(buffer, count); // Echo data ut på Serial
        lagreTilSD(buffer, count);  // Lagrer data til SD-kort

        noCharCount = 0; // Resetter teller for runder uten innkommende data
      }
      noCharCount++;

      // Timeout-funksjon i tilfelle klient ikke kobler fra server
      if (noCharCount > 1000)
      {
        Serial.println();
        Serial.println("Timeout");
        client.stop();
      }
    }
  }
}

void printWiFiStatus() 
{
  // Print SSID for aksesspunkt
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  //IP addresse for server
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

// Funksjon for å lagre data til SD-kort
void lagreTilSD(byte buffer[160], int count)
{
  File dataFil = SD.open(fileName, FILE_WRITE);

  if(dataFil)
  {
    dataFil.write(buffer, count);
    dataFil.close();
  }
  else Serial.println(F(errorSD));
}

// Testfunksjon for å verifisere at data blir skrevet til SD-kort
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
