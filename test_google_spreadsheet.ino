#include <Adafruit_AHTX0.h>

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_Sensor.h>

#include "time.h"
#include <ESP_Google_Sheet_Client.h>


#include <GS_SDHelper.h>

#define WIFI_SSID "X9"
#define WIFI_PASSWORD "ivan2004"

#define PROJECT_ID "rugged-weft-404613"


#define CLIENT_EMAIL "iot-temp-log@rugged-weft-404613.iam.gserviceaccount.com"

const char PRIVATE_KEY[] PROGMEM = "-----BEGIN PRIVATE KEY-----\nMIIEvAIBADANBgkqhkiG9w0BAQEFAASCBKYwggSiAgEAAoIBAQCo5fikqeA6/i3r\nKE3iM47tl0uUJ5NQ1R5cEYKTQCBSyspd3b5tSGMUs61lj3/4lAJNh5vTPlMbg+xX\nifm+w5olMZO1v7TWC4uHpDg9WKACsU9JuBbZtEFpXlZ68DHUtJH8Q9Xg24ChkCn/\nbsSZp+hcS7eHt5TgJm/Zw2PbuPU2yH2mYv44KEgRnV/pxdOebHvIywKlq4gnO+Vb\nHtTIaelLc4VDet8Vku23xfO995aKj/r9T6zkoYZebHc1GmF0pMfjlyJGCtgHcvF0\ngIs15KvpZISrwqfsMjIEcXxMtHqfZy2B8B57DrJWqPwCSc/DKiXsT08V8d1kza4X\n9q2jUzPnAgMBAAECggEAHsW7riCnUohCDIDM4+Q5687vfcBSJl2WCPdFv5dk7EC3\nay++BMI2Dy0HcCQbkEpDOyX0faHy2WYJVw+z+Y5A1Pr56xjRFViIJQAGISM3EBaH\nGCrauvKNImoGPqLpqVP27yva2XuUrZsLE9d2KXq+qqY++5K2pLpMEbR4Sq2hu8ey\nQS4AIGT9MW5btteR/b3dBVbEMUc5cpQ/mBUBppvFEi75kn5RxygXgqcwSYUa9Ba3\n9UlgRUX7ZKPHPl9ZdLJzLMt24FqfzlQ+hHYe30wdEGLwc4OkVSPInubaktX5JnPn\n9v9y1LawDJmt6tx+QyNemNgsOMvI4MSVKZxB84UeJQKBgQDr3rB3mB/hwyxm67vT\nmDuO3Rqrl9z/zRexPXkTbdb/q2zhEpXS7p/7kD/2MTSd/XYxjfYsYhTvxgIdLGsU\nNcDDPsLa+cPnRz4X9yCTpVxfGEU3w63zuh3wwrK1ZeS0Pk8Xf1hW3/+N4wmwblUD\neuAiFQpNENygVAn5OQS2B3/ddQKBgQC3UBSFGC2Vtg7XHQjgymjo0kQXNKr7Gezt\nsLMmavSZNL2yv8rPbMUdd8oFCd08KuoKLrvuHe1y8Er3cMWwO0I6BfgfNbDAmS9q\n/EXr5HgKg8oXkNSMjvQBDUWydy9lYJFOtZVkkpGTVM7/f59muFGyMbT22bidleqL\nwZiVhDKUawKBgDqAlmZjXe0CGr5AU+rshEEwCBBtb0wnE2xLof82rC/n1e6RTIXv\n19Bue97VX4acOjeaTWe+wBCknRudS8XMe04x/+SONI/ltLn++p6EUj/n5ispprif\n/hMcR3UEm9A08CsU09VgY6NOn7WZ+A4P6Q2rCE4LE53UmlCtWsTHYWiNAoGAD9wH\nynQLDZbQtd+3MuuQJAR9C5zs5giaLP5KoZpDSzX664oYW9XHQANrIQB3eYsk8HiW\nncVNE9KiScIh+FtIIYj+rISSpDccmR72VbBQimhYha1sFjUP+z9Qtl4FZbnzGGIx\naKAdpCeEOSqredGcj9QpGepARLIBttGrB9SGI70CgYADthCBYiSkUg6fYCKTVMKx\nsorQUBi/tz8d+BwAtWWvYKfDjDOidWSuQEvVECCE7lrKqrIsXtAMgQHgke75PjVF\nZeqN/h0ilILi/VAlQBa2uNswvy7AAi6Wb4bouHCWR5kVljpuqMsUi+SoUvuJPkEY\nMBlx4aJr+rJ+vB1eTV6g/Q==\n-----END PRIVATE KEY-----\n";

const char spreadsheetId[] = "1IMfhO3OuDmLisBGgU3a_EC0ho5B3x8xSsVI9lucDC7g";


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
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

void setup(){

    Serial.begin(115200);
    Serial.println();
    Serial.println();

    //Configure time
    configTime(0, 0, ntpServer);

    
    if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }

    GSheet.printf("ESP Google Sheet Client v%s\n\n", ESP_GOOGLE_SHEET_CLIENT_VERSION);

    // Connect to Wi-Fi
    WiFi.setAutoReconnect(true);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(1000);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
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

        Serial.println("\nAppend spreadsheet values...");
        Serial.println("----------------------------");

        FirebaseJson valueRange;
        sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

       
        
        epochTime = getTime();

        valueRange.add("majorDimension", "COLUMNS");
        valueRange.set("values/[0]/[0]", epochTime);
        valueRange.set("values/[1]/[0]", temp.temperature);
        valueRange.set("values/[2]/[0]", humidity.relative_humidity);
        

       
        bool success = GSheet.values.append(&response /* returned response */, spreadsheetId /* spreadsheet Id to append */, "Sheet1!A1" /* range to append */, &valueRange /* data range to append */);
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
