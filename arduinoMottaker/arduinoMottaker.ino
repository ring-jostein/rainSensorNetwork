#include <SPI.h>
#include <WiFiNINA.h>
#include <SD.h>

#define chipSelect 4  //pinne for kommunikasjon med SD-kort
#define fileName "datalog.txt"
#define errorSD "Feil: Kan ikke åpne datalog.txt fra SD-kort"
#define ssid "abc123"
#define pass "12345678"

int status = WL_IDLE_STATUS;

WiFiServer server(2323);
//WiFiClient clients[maxClients];

void setup() 
{
  Serial.begin(115200);   // initialiserer serial for debugging

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
  WiFiClient client = server.available();

  if (client)
  {
    //Serial.println(F("Ny klient"));
    while (client.connected())
    {
      if (client.available())
      {
        byte buffer[80];
        int count = client.read(buffer, 80);
        Serial.write(buffer, count);  //kun for testing
        Serial.println();  //kun for testing
  
        lagreTilSD(buffer, count);
        lesFraSD();
      }
    }
    //lukker koblingen:
    client.stop();
    Serial.println("Klient frakoblet");
  }

  /*
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
  */
}

void printWiFiStatus() 
{
  // Print SSID for aksesspunkt
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  //IP addresse for WiFi
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
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
