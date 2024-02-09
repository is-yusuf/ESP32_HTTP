#include <WiFi.h>
#include "esp_wpa2.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>


AsyncWebServer server(80);



#define EAP_IDENTITY "eduroam"
#define EAP_USERNAME ""
#define EAP_PASSWORD ""
String apiKey = "";

int counter = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  connect_wifi();
  WebSerial.begin(&server);
  WebSerial.msgCallback(recvMsg);
  server.begin();
}
void loop() {
  if (Serial.available() > 0) {
    Serial.println(WiFi.localIP()); 

    String message = Serial.readStringUntil('\n');
    makeHTTPRequest(message);
  }
  delay(100);
}

void connect_wifi(){
  Serial.print("Connecting to network: ");
  Serial.println(EAP_IDENTITY);
  WiFi.disconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(EAP_IDENTITY, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if(counter>=60){ 
      ESP.restart();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address set: "); 
  Serial.println(WiFi.localIP()); 
  
}
String makeHTTPRequest(String userMessage) {
  if (WiFi.status() == WL_CONNECTED) { 
    HTTPClient http;

    String postData = "{\"model\": \"gpt-3.5-turbo\", \"messages\": [{\"role\": \"user\", \"content\": \"" + userMessage + "\"}]}";

    http.begin("https://api.openai.com/v1/chat/completions");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + apiKey);

    int httpCode = http.POST(postData);
    String payload = http.getString();

  const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + 2*JSON_OBJECT_SIZE(2) + 60;
  DynamicJsonDocument doc(capacity);
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return "";
  }
  const char* content = doc["choices"][0]["message"]["content"];
    http.end();
  Serial.println(content);
  return content;

  }
  else {
    Serial.println("WiFi not connected");
  }
}

void recvMsg(uint8_t *data, size_t len){
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(makeHTTPRequest(d));
}
