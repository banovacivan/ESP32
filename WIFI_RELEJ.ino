/*
 WiFi Web Server LED + MIT aplikacija 

 http://adresa/H -> ON
 http://adresa/L -> OFF

 WPA2 encryption -> za ostale -> Wifi.begin() -> Wifi.setMinSecurity() 
 ESP32 DevModule
*/

#include <WiFi.h>

const char* ssid     =""//ime Wi-Fi;
const char* sifra =""//sifra;

WiFiServer server(80);

void setup()
{
    Serial.begin(115200);
    pinMode(26, OUTPUT);   //PIN mode na OUTPUT -> LED-ica na pin 26

    delay(10);

    //spajanje

    Serial.println();
    Serial.println();
    Serial.print("Spajanje na: ");
    Serial.println(ssid);

    WiFi.begin(ssid, sifra);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi spojen");
    Serial.println("IP adresa: ");
    Serial.println(WiFi.localIP());
    
    server.begin();

}

void loop(){
 WiFiClient client = server.available();  

  if (client) {                           
    Serial.println("New Client.");          
    String currentLine = "";               
    while (client.connected()) {            
      if (client.available()) {            
        char c = client.read();            
        Serial.write(c);                    
        if (c == '\n') {                    
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println();
            client.print(" <a href=\"/H\"> link </a> ON <br>");
            client.print(" <a href=\"/L\"> link </a> OFF <br>");

        
            client.println();
            break;
          } else {    
            currentLine = "";
          }
        } else if (c != '\r') {  
          currentLine += c;     
        }
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(26, HIGH);               
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(26, LOW);                
        }
      }
    }
    client.stop();
    Serial.println("Client Disconnected.");
  }
}
