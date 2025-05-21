#include "wifi_config.h"
#include "web_serverin.h"

bool conexcion = false;
bool allowcontrol = false;
int hours = 0, minutes = 0, seconds = 0, days = 0;
unsigned long timeNow = 0, timeLast = 0, startTime = 0;


void setup() {
  Serial.begin(115200); //inicia serial

  // Inicializa conexões de rede
  startWiFi();        // Função adaptada do wifi_config.h
  startOTA();         // Inicializa OTA
  startSPIFFS();      // Inicializa sistema de arquivos
  startWebSocket();   // Inicializa WebSocket
  startMDNS();        // Inicializa mDNS
  startServer();      // Função adaptada do web_server.h

}

void loop() {
  // Atualizações de rede necessárias
  server.handleClient();          // Trata requisições HTTP
  webSocket.loop();               // Atualiza WebSocket
  ArduinoOTA.handle();            // Mantém OTA atualizado

}