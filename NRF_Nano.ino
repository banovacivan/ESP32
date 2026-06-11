#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

void setup() {
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN); 
  radio.stopListening();
}

void loop() {
  int potValue = analogRead(A0);
  int angleValue = map(potValue, 0, 1023, 0, 180); 
  
  radio.write(&angleValue, sizeof(angleValue));
  delay(10); 
}
