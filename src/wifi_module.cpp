
#include <Arduino.h>

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include <wifi_module.h>

// Set web server port number to 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Variable to store the HTTP request
String header;

String get_current_pressure_reading_bar();

void setup_wifi_module() {

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

    Serial.println("Setting AP (Access Point)…");
    WiFi.softAP("KMS - Monster fog machine");

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html");
    });
    server.on("/logo", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/logo.jpg", "image/png");
    });
    server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", get_current_pressure_reading_bar().c_str());
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    });

  server.begin();
}

String get_current_pressure_reading_bar() {
    return String(5.0);
}


String generate_reading() {
  // Read temperature as Celsius (the default)
  float t = random(5, 10);
  if (isnan(t)) {
    Serial.println("Failed to read from BME280 sensor!");
    return "";
  }
  else {
    return String(t);
  }
}
