#include <avr/sleep.h> //this AVR library contains the method that controls the sleep modes
#define interuptPin 2 //Pin we are going to use to wake up the Arduino

void setup() {
  // put your setup code here, to run once:
Serial.begin(115200); //Start serial communication
pinMode(LED_BUILTIN, OUTPUT); //We use led on pin 13 to indicate when Arduino is a sleep
pinMode(interuptPin, INPUT_PULLUP); //Set pin d2 to input using the buildtin pullup resistor
digitalWrite(LED_BUILTIN, HIGH); //Turning LED on
}

void loop() {
  // put your main code here, to run repeatedly:
delay(5000); //Wait 5 seconds before going to sleep
goingToSleep();
}

void goingToSleep()
{
  sleep_enable(); //Enable sleep
  attachInterrupt(0, wakeUp, LOW); //Attaching a interrupt to pin d2
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Setting the sleep mode, in our case full sleep
  digitalWrite(LED_BUILTIN, LOW); //Turning LED off
  Serial.println("Going to sleep..."); //Print message befor going to sleep
  delay(1000); //Wait a second to allow the led to be turned off before going to sleep
  sleep_cpu(); //Activating sleep mode
  Serial.println("just woke up!"); //Next line of code executed after the interrupt
  digitalWrite(LED_BUILTIN, HIGH); //Turning LED on
}

void wakeUp()
{
  Serial.println("Interrupt Fired"); //Print message to serial monitor
  sleep_disable(); //Disable sleep mode
  detachInterrupt(0); //Remove the intertupt from pin 2
}
