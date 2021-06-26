#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

#include <wifi_module.h>
#include <control.h>

#define SSID "KMS - Monster fog machine"

DNSServer dnsServer;
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

class CaptiveRequestHandler : public AsyncWebHandler {
public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
        return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html");
    }
};

void setup_wifi_module() {
    if (!SPIFFS.begin()) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        while(true) {}; // stop further execution
    }

    Serial.println("Setting up AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SSID);

    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    //---------------------UI RELATED METHOD--------------------------
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html");
    });

    server.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/logo.png", "image/png");
    });

    server.on("/jquery-3.5.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/jquery-3.5.1.min.js");
    });

    server.on("/bootstrap.min.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/bootstrap.min.js");
    });

    server.on("/bootstrap.min.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/bootstrap.min.css");
    });

    server.on("/favicon-16x16.png", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/favicon-16x16.png", "image/png");
    });
    //--------------------CONTROL METHODS-----------------------------
    server.on("/start-manual", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Manual control mode started");
        controlMode = CONTROL_MODE_MANUAL;

        request->send(200, "text/plain", "OK");
    });

    server.on("/stop-manual", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Manual control mode stopped");
        controlMode = CONTROL_MODE_NONE;

        request->send(200, "text/plain", "OK");
    });

    server.on("/start-auto", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Automatic control mode started");
        controlMode = CONTROL_MODE_AUTO;
        
        if (request->hasParam("timeBfrSpray")) {
            String paramStr = request->getParam("timeBfrSpray")->value();
            float timeBfrSpray = atof(paramStr.c_str());
            waitingTime = timeBfrSpray * 60 * 1000;
        }
        if (request->hasParam("sprayTime")) {
            String paramStr = request->getParam("sprayTime")->value();
            float sprayTime = atof(paramStr.c_str());
            activationTime = sprayTime * 1000;
        }
        request->send(200, "text/plain", "OK");
    });

    server.on("/stop-auto", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Automatic control mode stopped");
        controlMode = CONTROL_MODE_NONE;

        request->send(200, "text/plain", "OK");
    });

    server.on("/heater-status", HTTP_GET, [](AsyncWebServerRequest *request) {
        bool heaterState = false;
        if (request->hasParam("sensor")) {
            String sensorNrStr = request->getParam("sensor")->value();
            int sensorNr = atoi(sensorNrStr.c_str());
            
            if (sensorNr == 1) {
                heaterState = heater1state;
            } else if (sensorNr == 2) {
                heaterState = heater2state;
            } else if (sensorNr == 3) {
                heaterState = heater3state;
            }
        }

        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", heaterState ? "true" : "false");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    server.on("/liquid-status", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", fluidTankState ? "true" : "false");
        response->addHeader("Access-Control-Allow-Origin", "*");

        request->send(response);
    });

    server.on("/mode-status", HTTP_GET, [](AsyncWebServerRequest *request) {
        String currMode;
        if (controlMode == CONTROL_MODE_NONE) {
            currMode = "none";
        } else if (controlMode == CONTROL_MODE_MANUAL) {
            currMode = "manual";
        } else if (controlMode == CONTROL_MODE_AUTO) {
            currMode = "auto";
        }

        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", currMode);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    server.onNotFound([](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html");
    });
    //------------------------------------------------------------
    dnsServer.start(53, "*", WiFi.softAPIP());
    server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); //only when requested from AP
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
