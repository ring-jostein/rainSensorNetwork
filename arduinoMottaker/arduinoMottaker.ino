#include <SPI.h>
#include <WiFiNINA.h>
#include <SD.h>

#define chipSelect 4  //pinne 4 for kommunikasjon med SD-kort for MKR, 10 for Uno
#define fileName "datalog.txt"
#define errorSD "Feil: Kan ikke åpne datalog.txt fra SD-kort"
#define ssid "abc123"
#define pass "12345678"

int status = WL_IDLE_STATUS;
WiFiServer server(2323);
//WiFiClient clients[4];

void setup() 
{
  Serial.begin(2000000);   // initialiserer serial for debugging

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

  int noCharCount = 0;
  if (client)
  {
    while (client.connected())
    {
      while (client.available())
      {
        byte buffer[160];
        int count = client.read(buffer, 160);
        Serial.write(buffer, count);
        //lagreTilSD(buffer, count);

        noCharCount = 0;
      }
      delay(10);
      noCharCount++;

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
