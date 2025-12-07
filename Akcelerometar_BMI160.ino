#include <BMI160Gen.h>
#include <Wire.h>
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
  Wire.write(0x11);
  Wire.write(0x7E);  //registar
  Wire.endTransmission();
  delay(100);

  autoCalibrateAccelerometer();
  Serial.println ("senor je inicijaliziran i kalibriran");

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

  float ax_ms = ax * (9.81 / 16384); // za 2g
  float ay_ms = ay * (9.81 / 16384);
  float az_ms = az * (9.81 / 16384);

  Serial.printf("akceleracija u m/s^2: ");
  Serial.printf("%.2f " ,ax_ms);
  Serial.printf("%.2f " ,ay_ms);
  Serial.printf("%.2f\n" ,az_ms);
  delay(50);
}

void autoCalibrateAccelerometer() {
  Wire.beginTransmission(0x68);
  Wire.write(0x7E); 
  Wire.write(0x37); //start
  delay(100);
  delay(1000);
  Serial.println("kalibracija kompletirana");
}
