#include <Arduino.h>

#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include <wifi_module.h>

const byte DNS_PORT = 53;
IPAddress apIP(192,168,4,1); // The default android DNS
DNSServer dnsServer;

// Set web server port number to 80
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Variable to store the HTTP request
String header;

String get_current_pressure_reading_bar();

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
  }
};

void setup_wifi_module() {

  

  // Initialize SPIFFS
  if(!SPIFFS.begin()){
    Serial.println("An Error has occurred while mounting SPIFFS");
  }

    Serial.println("Setting AP (Access Point)…");
    WiFi.mode(WIFI_AP); 
    WiFi.softAP("KMS - Monster fog machine");

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/index.html");
    });
    server.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/logo.png", "image/png");
    });
    server.on("/jquery-3.5.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/jquery-3.5.1.min.js");
    });
    server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/bootstrap.min.js");
    });
    server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/bootstrap.min.css");
    });
    server.on("/favicon-16x16.png", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/favicon-16x16.png", "image/png");
    });

    server.on("/start-manual", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("MANUAL_START");
      request->send(200, "text/plain", "OK");
    });
    server.on("/stop-manual", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("MANUAL_STOP");
      request->send(200, "text/plain", "OK");
    });
    server.on("/start-auto", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("AUTO_START");
      request->send(200, "text/plain", "OK");
    });
    server.on("/stop-auto", HTTP_GET, [](AsyncWebServerRequest *request){
      Serial.println("AUTO_STOP");
      request->send(200, "text/plain", "OK");
    });
    server.on("/heater-status", HTTP_GET, [](AsyncWebServerRequest *request){
      bool heaterState;
      if (request->hasParam("sensor")) {
        String sensor = request->getParam("sensor")->value();
        int sensorNr = atoi(sensor.c_str());
        if(sensorNr == 1) {
          heaterState = false;
        } else if(sensorNr == 2) {
          heaterState = true;
        } if(sensorNr == 3) {
          heaterState = false;
        }
      }
      String state = heaterState ? String("true") : String("false");
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", state);
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    });
    server.on("/liquid-status", HTTP_GET, [](AsyncWebServerRequest *request){
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", String("true"));
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    });
    server.on("/mode-status", HTTP_GET, [](AsyncWebServerRequest *request){
      String mode;
      
      int tmp = 1;
      if (tmp == 1) {
        mode = String("none");
      } else if (tmp == 2) {
        mode = String("manual");
      } else if (tmp == 3) {
        mode = String("auto");
      }
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", mode);
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    });

// replay to all requests with same HTML
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html");
  });

  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  server.begin();
}

void handleRequests() {
  dnsServer.processNextRequest();
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
