#define PIN_ANALOG_IN 2
int digitalPin =  15;

int analogVal = 0;
int adcVal = 0;

void setup() {
  Serial.begin(9600);
  pinMode(digitalPin, INPUT); 
}

void loop() {
  int digitalVal = digitalRead(digitalPin);  //Read 
  int adcVal = analogRead(PIN_ANALOG_IN);
  int dacVal = map(adcVal, 0, 4095, 0, 255);
  double voltage = adcVal / 4095.0 * 3.3;
  double miligrami = voltage*0.4;
  double promili = miligrami * 2.1;
  Serial.printf("Promili: %.2f‰\n", promili);
  delay(1000); //Delay time 100 ms
}
