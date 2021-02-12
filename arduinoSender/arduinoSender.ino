#include <DS3232RTC.h>
#include <TimeLib.h>

tmElements_t tm;

void setup() // put your setup code here, to run once
{
  Serial.begin(9600);
  setTime(15, 35, 00, 10, 02, 2021);
  RTC.begin();  //initialiserer I2C bus
  RTC.set(now());  //setter RTC fra systemtid
}

void loop() // put your main code here, to run repeatedly
{
  RTC.read(tm);
  int t = RTC.temperature();
  float celcius = t / 4.0;
  Serial.print(tm.Hour, DEC);
  Serial.print(':');
  Serial.print(tm.Minute, DEC);
  Serial.print(':');
  Serial.print(tm.Second, DEC);
  Serial.print(" Temperatur: ");
  Serial.print(celcius);
  Serial.println(" Celcius");
  delay(1000);
}  
