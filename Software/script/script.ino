#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#ifndef STASSID
#define STASSID "JojoNet"
#define STAPSK "Sennahoj08!?"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

String serverPath = "http://192.168.1.111:3000";

const int adc = A0;
int adcValue = 0;

void setup() {
  // Set D1 as Output pin
  pinMode(5, OUTPUT);

  digitalWrite(5, HIGH)

  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to Wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {

  adcValue = analogRead(adc);

  Serial.print("ADC Value: ");
  Serial.print(adcValue);
  Serial.println("V");

  if (adcValue * (1000 / (1000 + 15000 + 20000)) < 2.7) {
    digitalWrite(5, LOW)
  }

  WiFiClient client;
  HTTPClient http;

  static bool wait = false;

  http.begin(client, serverPath.c_str());

  http.addHeader("Content-Type", "application/json");

  String payload = "{\"voltage\":\"" + String(adcValue) + "\"}";

  Serial.println(payload);

  int httpResponseCode = http.POST(payload);

  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  // wait for data to be available
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      delay(60000);
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  Serial.println("receiving from remote server");
  // not testing 'client.connected()' since we do not need to send data here
  while (client.available()) {
    char ch = static_cast<char>(client.read());
    Serial.print(ch);
  }

  // Close the connection
  Serial.println();
  Serial.println("closing connection");
  client.stop();

  if (wait) {
    delay(300000);  // execute once every 5 minutes, don't flood remote service
  }
  wait = true;
}
