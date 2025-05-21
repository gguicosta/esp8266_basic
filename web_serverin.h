#ifndef WEB_SERVERIN_H
#define WEB_SERVERIN_H

#include <ESPmDNS.h>
#include "SPIFFS.h"
#include <WebServer.h>
#include <WebSocketsServer.h>

// Adicione estas declarações para as funções que estavam faltando
void handleFileUpload();
void handleNotFound();
bool handleFileRead(String path);
String getContentType(String filename);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght);

// Adicione estas variáveis globais que estavam faltando
extern bool conexcion;
extern bool allowcontrol;
extern int hours, minutes, seconds, days;
extern unsigned long timeNow, timeLast, startTime;
extern String receivedData;
/*____________________________funções_secundárias___________________________________________*/
String getContentType(String filename) {
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
  return "";
}
/*___________SPIFFS_______________________________________________SERVER_HANDLERS__________________________________________________________*/
void handleNotFound() {
  if (!handleFileRead(server.uri())) {        // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }

  String path = server.uri(); // Important!
  if (!SPIFFS.exists(path)) {
    server.send(404, "text/plain", "Path " + path + " not found. Please double-check the URL");
    return;
  }
  String contentType = "text/plain";
  if (path.endsWith(".css")) {
    contentType = "text/css";
  }
  else if (path.endsWith(".html")) {
    contentType = "text/html";
  }
  else if (path.endsWith(".js")) {
    contentType = "application/javascript";
  }
  File file = SPIFFS.open(path, "r");
  server.streamFile(file, contentType);
  file.close();
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  return false;
}

void handleFileUpload() { // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(".gz")) {                         // The file server always prefers a compressed version of a file
      String pathWithGz = path + ".gz";                  // So if an uploaded file is not compressed, the existing compressed
      if (SPIFFS.exists(pathWithGz))                     // version of that file must be deleted (if it exists)
        SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");            // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {                                   // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location", "/success.html");     // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

void handleFornatSPIFFS() {
  // Next lines have to be done ONLY ONCE!!!!!When SPIFFS is formatted ONCE you can comment these lines out!!
  //Serial.println("Please wait 30 secs for SPIFFS to be formatted");

  SPIFFS.format();

  spiffstatus = "Spiffs formatted";
  Serial.println(spiffstatus);
  handleNotFound();
  server.send(303);                         // Send it back to the browser with an HTTP status 303 (See Other) to redirect
}
/*_____________________________WEBSOCKET______________________________________________________________*/
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
//       if (rodando = true){
//       conexcion = true;
//       allowcontrol = false;   }                  // Turn allowcontrol off when a new connection is established
//       timeNow = millis()/1000; // the number of milliseconds that have passed since boot
//       seconds = timeNow - timeLast;//the number of seconds that have passed since the last time 60 seconds was reached.
// //      Serial.printf("timer rodando: hora %d minuto %d segundo %d\n", hours, minutes, seconds);
//       if (seconds > 59) {
//         int x = seconds/60;
//         seconds = seconds - x*60;
//         timeLast= timeLast+(x*60); 
//         minutes = minutes + x;
// //        attarq();
//       }
//       if (minutes > 59){ 
//         int x = minutes/60;
//         minutes = minutes - x*60;
//         hours = hours + x;
//       }
//       if (hours >= 24){
//         int x = hours/24;
//         hours = hours-x*24;
//         days = days + x;
//         }
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        if (rodando = true){
        conexcion = false; allowcontrol = false;                  // Turn allowcontrol off when a new connection is established
        }
      }
      break;
    case WStype_TEXT: {  // Quando recebe texto como reage?
     /*Duas formas de tratar os textos vindos método 
     a)Recebe o texto vindo do websocket, deste pega o primeiro char 
      armazenado em payload[0] e define o que será feito em um função 
      exemplos a seguir
 */   Serial.printf("[%u] get Text: %s\n", num, payload);
      if (payload[0] == '#') {            // we get RGB data
      } else if (payload[0] == 'm') {    
        //receivedData = String((char*)payload);
        Serial.println("Dados recebidos: " + receivedData);
      } else if (payload[0] == 'R') {                      // the browser sends an R when the allowcontrol effect is enabled
        allowcontrol = false;

      } else if (payload[0] == 'T') {    // the browser sends an N when the allowcontrol effect is disabled
          allowcontrol = false;
          rodando= false;
          
          //método b) Recebendo json
        //   String message = String((char *) &payload[0]);
        //   Serial.println("Recebido: " + message);
          
        //   StaticJsonDocument<200> doc;
        //   DeserializationError error = deserializeJson(doc, message);
        //   // Se a mensagem não der erro colocar qual a informação advinda
        //   if (!error) {
        //     const char* tipo = doc["tipo"];
        //     int valor = doc["valor"];
        //     if (strcmp(tipo, "velocidade") == 0) {
        //         velocidade = valor;
        //     } else if (strcmp(tipo, "direcao") == 0) {
        //         direcao = valor;
        //     }
        //   } else {
        //     Serial.println("Erro ao analisar JSON");
        // }
        // break;
      }
    }
    break;
  }
}

#endif
