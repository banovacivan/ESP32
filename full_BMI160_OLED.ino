#include <BMI160Gen.h>            //https://github.com/hanyazou/BMI160-Arduino
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <stdint.h>
#define visina 64
#define sirina 128
Adafruit_SSD1306 display(sirina, visina, &Wire, -1);
// BMI -> ESP32 (Dev Module)
//3.3V -> 3.3V
//GND -> GND
// SCL -> SCL (PIN22)
// SDA -> SDA (PIN21)
// SAO -> 3.3V (0x69) / GND (0x68)


void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
  Serial.println(F("SSD1306 FAIL"));
  for(;;);
  }
  BMI160.setAccelerometerRange(2);
  if (!BMI160.begin(BMI160GenClass::I2C_MODE, 0x68)) {
    Serial.println("failed!");
    while (1);
  }
  Serial.println("uspijeh");
}

void loop() {
  int gx, gy, gz; 
  int ax, ay, az; 

  BMI160.readGyro(gx, gy, gz);
  BMI160.readAccelerometer(ax, ay, az);
  
  float gx_g = gx / 16384.0;
  float gy_g = gy / 16384.0;
  float gz_g = gz / 16384.0;

  float ax_g = ax *(9.81/ 16384.0);
  float ay_g = ay *(9.81/ 16384.0);
  float az_g = az *(9.81/ 16384.0);

  float pitch = atan2(ay_g, sqrt(ax_g * ax_g + az_g * az_g)) * 180.0 / PI;
  float roll = atan2(-ax_g, az_g) * 180.0 / PI;

  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 0);
  display.println("m/s^2");
  display.println(ax_g);
  display.println(ay_g);
  display.println(az_g);

  display.setCursor(50, 0);
  display.println("°");
  display.setCursor(50, 10);
  display.println(pitch);
  display.setCursor(50, 20);
  display.println(roll);
  
  display.display();
  delay(10);
  display.clearDisplay();


  delay(100); 
}