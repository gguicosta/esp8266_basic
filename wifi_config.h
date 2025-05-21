#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <WiFi.h>
#include <WiFiMulti.h>
#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include "SPIFFS.h"
#include <WebServer.h>
#include <WebSocketsServer.h>

WiFiMulti wifiMulti;              // Create an instance of the WiFiMulti class, called 'wifiMulti'

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght);
void handleFileUpload();
void handleNotFound();

WebServer server(80);             // create a web server on port 80
WebSocketsServer webSocket(81);   // create a websocket server on port 81

File fsUploadFile;                // a File variable to temporarily store the received file

const char* spiffstatus = "";

const char *ssid = ""; // The name of the Wi-Fi network that will be created
const char *password = "12345678";   // The password required to connect to it, leave blank for an open network

const char *OTAName = "";      // A name and a password for the OTA service
const char *OTAPassword = "31415";

const char* mdnsName = "endereço_web"; //--> endereço_web.local    // Domain name for the mDNS responder
String receivedData = "";            // Defina uma variável global para armazenar os dados do payload
bool rodando = 0;

String formatBytes(size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < (1024 * 1024)) return String(bytes / 1024.0) + " KB";
  else return String(bytes / 1024.0 / 1024.0) + " MB";
}
void startWiFi() { // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  WiFi.softAP(ssid, password);       // Start the access point
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started\r\n");

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  wifiMulti.addAP("Rede", "Senha");   // add Wi-Fi networks you want to connect to
  //wifiMulti.addAP("Nome", "Senha");
  //wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting");
  while (wifiMulti.run() != WL_CONNECTED && WiFi.softAPgetStationNum() < 1) {// Wait for the Wi-Fi to connect
    delay(250);
    Serial.print('.');
  }
  Serial.println("\r\n");
  if (WiFi.softAPgetStationNum() == 0) {  // If the ESP is connected to an AP
    Serial.print("Connected to ");
    Serial.println(WiFi.SSID());             // Tell us what network we're connected to
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP());            // Send the IP address of the ESP32 to the computer
  } else {                                   // If a station is connected to the ESP SoftAP
    Serial.print("Station connected to ESP32 AP");
  }
  Serial.println("\r\n");
}

void startOTA() { // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready\r\n");
}

void startSPIFFS() { // Start the SPIFFS and list all contents
  if(!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  
  Serial.println("SPIFFS started. Contents:");
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  while(file) {
    Serial.printf("FS File: %s, size: %s\r\n", file.name(), formatBytes(file.size()).c_str());
    file = root.openNextFile();
  }
  Serial.println();

  File indexFile = SPIFFS.open("/index.html", "r");
  if(!indexFile) {
    Serial.println("Failed to open index.html");
  } else {
    indexFile.close();
  }
}

void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start the websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}

void startMDNS() { // Start the mDNS responder
  if(!MDNS.begin(mdnsName)) {
    Serial.println("Error setting up MDNS responder!");
  } else {
    Serial.print("mDNS responder started: http://");
    Serial.print(mdnsName);
    Serial.println(".local");
  }
}

void handleRoot() {
  File file = SPIFFS.open("/index.html", "r");
  if (!file) {
    server.send(500, "text/plain", "Problem with filesystem!\n");
    return;
  }
  server.streamFile(file, "text/html");
  file.close();
}

void startServer() { // Start a HTTP server with a file read handler and an upload handler
  server.on("/edit.html", HTTP_POST, []() {
    server.send(200, "text/plain", "");
  }, handleFileUpload);

  server.on("/download", HTTP_GET, []() {
    String data = "Dados salvos no ESP32";
    server.send(200, "text/plain", data);
  });

  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started.");

  server.on("/state", HTTP_GET, []() {
    if (!SPIFFS.exists("/state.txt")) {
      server.send(404, "text/plain", "Arquivo não encontrado");
      return;
    }

    File file = SPIFFS.open("/state.txt", "r");
    if (!file) {
      server.send(500, "text/plain", "Erro ao abrir o arquivo");
      return;
    }

    String content = file.readString();
    server.send(200, "application/json", content);
  });
}
#endif