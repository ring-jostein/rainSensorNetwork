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
WiFiClient clients[4];

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
  
  if (client)
  {
    //Serial.println(F("Ny klient"));
    while (client.connected())
    {
      if (client.available())
      {
        byte buffer[160];
        int count = client.read(buffer, 160);
        lagreTilSD(buffer, count);
        Serial.write(buffer, count);
      }
    }
    //lukker koblingen:
    client.stop();
    Serial.println("Klient frakoblet");
  }
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
