#include <SPI.h>
#include <WiFiNINA.h>
#include <SD.h>

//#define maxClients 4  //ESP-01 håndterer ikke fler enn 4 klienter
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
  Serial.begin(2000000);   // initialiserer serial for debugging

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
  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) 
  {
    Serial.println("Creating access point failed");
    // don't continue
    while (true);
  }

  // wait 10 seconds for connection:
  delay(10000);

  // starter server
  server.begin();
  Serial.println(F("Server etablert"));

  // print status
  printWiFiStatus();
}

void loop() 
{
  // Leter etter nye klienter og legger dem til et klientarray
  WiFiClient client = server.available();

  if (client)
  {
    Serial.println(F("Ny klient"));
    while (client.connected())
    {
      if (client.available())
      {
        byte buffer[160];
        int count = client.read(buffer, 160);
        lagreTilSD(buffer, count);
        
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
  
  //lesFraSD();

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
