#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN); // Use _HIGH if you have stable power
  radio.stopListening();
}

void loop() {
  int potValue = analogRead(A0);
  int angleValue = map(potValue, 0, 1023, 0, 180); // Map to servo degrees
  
  radio.write(&angleValue, sizeof(angleValue));
  delay(10); // Smooth out the transmission
}