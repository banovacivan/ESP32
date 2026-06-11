#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <ESP32Servo.h>

RF24 radio(2, 7); // CE, CSN pins for ESP32-C3
const byte address[6] = "00001";
Servo myServo;

void setup() {
  myServo.attach(3); // Signal on GPIO 3
  
  // Start SPI with C3 default pins: SCK=4, MISO=5, MOSI=6
  SPI.begin(4, 5, 6, 7); 
  
  if (!radio.begin()) {
    // If radio fails to start, you can add a serial print here to debug
  }
  
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
  Serial.begin(115200);
}

void loop() {
  if (radio.available()) {
    int receivedAngle = 0;
    radio.read(&receivedAngle, sizeof(receivedAngle));
    
    // Safety check to ensure the value is within servo range
    if (receivedAngle >= 0 && receivedAngle <= 180) {
      myServo.write(receivedAngle);
    }
  }
}