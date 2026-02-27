#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Adafruit_VL53L0X.h"
#include <STROJNO_inferencing.h> 

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// Reset pin je -1 ako tvoj ekran nema reset pin (većina SSD1306)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_VL53L0X lox = Adafruit_VL53L0X();

float prag_sigurnosti = 0.70;

void setup() {
    Serial.begin(115200);
    delay(1000); 
    Wire.begin(21, 22);
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("OLED nije pronaden na 0x3C"));
    } else {
        Serial.println(F("OLED spreman"));
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
    }
    if (!lox.begin()) {
        Serial.println(F("Greška: VL53L0X nije pronađen!"));
    } else {
        Serial.println(F("Senzor spreman"));
    }
    lox.setMeasurementTimingBudgetMicroSeconds(20000);
    delay(1000);
    Serial.println("Sustav spreman za rad.");
}

void loop() {
    float buffer[EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE];

    for (size_t ix = 0; ix < EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE; ix++) {
        unsigned long next_sampling_time = millis() + (unsigned long)EI_CLASSIFIER_INTERVAL_MS;      
        VL53L0X_RangingMeasurementData_t measure;
        lox.rangingTest(&measure, false);
        
        if (measure.RangeStatus != 4) {
            buffer[ix] = (float)measure.RangeMilliMeter;
        } else {
            buffer[ix] = 1200.0f; 
        }
        while (millis() < next_sampling_time) { ; }
    }

    signal_t signal;
    numpy::signal_from_buffer(buffer, EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, &signal);  
    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

    if (res != EI_IMPULSE_OK) {
        Serial.printf("Greška u klasifikaciji (%d)\n", res);
        return;
    }

    bool gesta_prepoznata = false;

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        float vjerojatnost = result.classification[ix].value;
        String labela = String(result.classification[ix].label);

        if (vjerojatnost >= prag_sigurnosti && labela != "NULL") {
            display.clearDisplay();
            display.setTextColor(SSD1306_WHITE);
            
            display.setTextSize(2);
            display.setCursor(0, 5);
            display.print("GESTA:");
            
            display.setTextSize(2);
            display.setCursor(0, 30);
            display.println(labela);
            
            display.display();
            Serial.print("Detektirano: "); Serial.println(labela);
            gesta_prepoznata = true;
            delay(100);
        }
    }
    if (!gesta_prepoznata) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setCursor(30, 50);
        display.println("Osluskujem...");
        display.display();
    }
}