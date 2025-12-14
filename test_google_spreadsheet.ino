#include <Adafruit_AHTX0.h>

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>

#include "time.h"
#include <ESP_Google_Sheet_Client.h>


#include <GS_SDHelper.h>

#define WIFI_SSID ""
#define WIFI_PASSWORD ""

#define PROJECT_ID ""


#define CLIENT_EMAIL ""

const char PRIVATE_KEY[] PROGMEM = "";

const char spreadsheetId[] = "";


unsigned long lastTime = 0;  
unsigned long timerDelay = 450000;


void tokenStatusCallback(TokenInfo info);


Adafruit_AHTX0 aht;
const char* ntpServer = "pool.ntp.org";

unsigned long epochTime; 

unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return(0);
  }
  time(&now);
  return now;
}

void setup(){

    Serial.begin(115200);
    Serial.println();
    Serial.println();
    configTime(0, 0, ntpServer);   
    if (! aht.begin()) {
    Serial.println("fail/AHT");
    while (1) delay(10);
  }

    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);


    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
    Serial.print("Spajanje na  Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println();
    Serial.print(" IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    
    GSheet.setTokenCallback(tokenStatusCallback);

    
    GSheet.setPrerefreshSeconds(10 * 60);

    
    GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);
}

void loop(){
    
    bool ready = GSheet.ready();

    if (ready && millis() - lastTime > timerDelay){
        lastTime = millis();

        FirebaseJson response;
        FirebaseJson valueRange;
        sensors_event_t humidity, temp;
        aht.getEvent(&humidity, &temp);
      
        epochTime = getTime();

        valueRange.add("majorDimension", "COLUMNS");
        valueRange.set("values/[0]/[0]", epochTime);
        valueRange.set("values/[1]/[0]", temp.temperature);
        valueRange.set("values/[2]/[0]", humidity.relative_humidity);
        

       
        bool success = GSheet.values.append(&response, spreadsheetId , "Sheet1!A1", &valueRange);
        if (success){
            response.toString(Serial, true);
            valueRange.clear();
        }
        else{
            Serial.println(GSheet.errorReason());
        }
        Serial.println();
        Serial.println(ESP.getFreeHeap());
    }
}

void tokenStatusCallback(TokenInfo info){
    if (info.status == token_status_error){
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
        GSheet.printf("Token error: %s\n", GSheet.getTokenError(info).c_str());
    }
    else{
        GSheet.printf("Token info: type = %s, status = %s\n", GSheet.getTokenType(info).c_str(), GSheet.getTokenStatus(info).c_str());
    }
}

