#include <BMI160Gen.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdint.h>

// BMI -> ESP32 (Dev Module)
//3.3V -> 3.3V
//GND -> GND
// SCL -> SCL (PIN22)
// SDA -> SDA (PIN21)
// SAO -> 3.3V (0x69) / GND (0x68)

void setup() {
  Serial.begin(115200); 
  Wire.begin();
  //inicijaliziraj BMI160
  //SAO->GND i2c add=0x68 , SAO->3,3V i2c add=0x69
  Wire.beginTransmission(0x68);
  Wire.write(0x7E);  //registar
  Wire.write(0x11);
  Wire.endTransmission();
  delay(100);

  autoCalibrateAccelerometer();
  Serial.println ("seznor je inicijaliziran i kalibriran");

}

void loop() 
{
  int16_t ax,ay,az;
  Wire.beginTransmission(0x68);
  Wire.write(0x12);
  Wire.endTransmission(false);
  Wire.requestFrom(0x68, 6);
  
  if(Wire.available()==6)
  {
    ax = (Wire.read() | (Wire.read() << 8));
    ay = (Wire.read() | (Wire.read() << 8));
    az = (Wire.read() | (Wire.read() << 8));
  }

  float ax_g = ax / 16384.0; // za 2g
  float ay_g = ay / 16384.0;
  float az_g = az / 16384.0;

  float pitch = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / PI;
  float roll = atan2(-ax_g, az_g) * 180.0 / PI;

  
  Serial.print("Pitch: ");
  Serial.print(pitch, 2);
  Serial.print("°, Roll: ");
  Serial.print(roll, 2);
  Serial.println("°");
  delay(100);
}

void autoCalibrateAccelerometer() {
  Wire.beginTransmission(0x68);
  Wire.write(0x7E); 
  Wire.write(0x37); //start
  delay(100);
  delay(1000);
  Serial.println("kalibracija kompletirana");
}
