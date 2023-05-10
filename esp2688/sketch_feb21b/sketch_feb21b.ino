#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <StreamString.h>
#include <WebSocketsClient.h>

#include <ArduinoJson.h>
#include <Hash.h>

ESP8266WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial
StreamString arduinoJsonBuf;


void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  DynamicJsonDocument msg(1024);
  deserializeJson(msg, payload);
  String eventName = msg["eventName"]; 
  String eventPayload = msg["payload"]; 
  
  switch(type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

      webSocket.sendTXT(arduinoJsonBuf);
    }
      break;
    case WStype_TEXT:
      if(eventName == "switch-led"){
          if(eventPayload == "on"){
              USE_SERIAL.printf("Status: ON\n");
          };
          if(eventPayload == "off"){
              USE_SERIAL.printf("Status: OFF\n");
          };
      };
      break;
    case WStype_BIN:
      USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
      hexdump(payload, length);

      // send data to server
      // webSocket.sendBIN(payload, length);
      break;
        case WStype_PING:
            // pong will be send automatically
            USE_SERIAL.printf("[WSc] get ping\n");
            break;
        case WStype_PONG:
            // answer to a ping we send
            USE_SERIAL.printf("[WSc] get pong\n");
            break;
    }

}

void setup() {

  DynamicJsonDocument arduinoJson(1024);
  arduinoJson["eventName"] = "arduino-token";
  arduinoJson["payload"]   = "87e2f588-0dd8-4c0d-a12f-98e682c6ba54";
  
  
  serializeJson(arduinoJson, arduinoJsonBuf);


  // USE_SERIAL.begin(921600);
  USE_SERIAL.begin(115200);

//  Serial.setDebugOutput(true);
  USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for(uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }

  WiFiMulti.addAP("aMiN", "@7667minrezAei");

  //WiFi.disconnect();
  while(WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  // server address, port and URL
//  webSocket.begin("192.168.1.3", 80);
  webSocket.begin("smart.liara.run", 80);
  
  // event handler
  webSocket.onEvent(webSocketEvent);

  // use HTTP Basic Authorization this is optional remove if not needed
//  webSocket.setAuthorization("user", "Password");

  // try ever 5000 again if connection has failed
  webSocket.setReconnectInterval(5000);
  
  // start heartbeat (optional)
  // ping server every 15000 ms
  // expect pong from server within 3000 ms
  // consider connection disconnected if pong is not received 2 times
  webSocket.enableHeartbeat(15000, 3000, 2);

}

void loop() {
  webSocket.loop();
}
